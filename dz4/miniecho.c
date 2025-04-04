package git

import (
	"fmt"
	"os"
	"strconv"
	"strings"
)

type BlameResult struct {
	CommitHash string
	Author     string
	Committer  string
	LineCount  int
}

// BlameFile run `git blame <rev> --porcelain file`
func BlameFile(repoPath, revision, file string) ([]BlameResult, error) {
	output, err := RunGitCommand(repoPath, "blame", revision, "--porcelain", file)
	if err != nil {
		return nil, fmt.Errorf("failed to run git blame: %w", err)
	}

	if len(output) == 0 {
		return handleEmptyFile(repoPath, revision, file)
	}

	lines := strings.Split(output, "\n")

	var results []BlameResult
	var currentResult *BlameResult
	was := make(map[string]bool)

	for i := 0; i < len(lines); i++ {
		line := lines[i]

		// new block begins with hashcode
		if len(line) > 40 && strings.Count(line, " ") == 3 && strings.Count(line[:40], " ") == 0 && !was[line[:40]] {
			was[line[:40]] = true
			if currentResult != nil {
				results = append(results, *currentResult)
			}

			data := strings.Split(line, " ")
			a, ok := strconv.Atoi(data[3])
			if ok != nil {
				return []BlameResult{}, ok
			}
			currentResult = &BlameResult{
				CommitHash: data[0],
				LineCount:  a,
			}
			fmt.Fprintf(os.Stderr, "%s\n", line)
			// fmt.Fprintf(os.Stderr, "%s\n", lines[i+1])
			// fmt.Fprintf(os.Stderr, "%s\n", lines[i+5])
		} else if currentResult != nil && strings.HasPrefix(line, "\t") {
			// it is code string
			//currentResult.LineCount++
		} else if currentResult != nil && strings.HasPrefix(lines[i], "author ") {
			currentResult.Author = strings.TrimSpace(lines[i][len("author "):])
		} else if currentResult != nil && strings.HasPrefix(lines[i], "committer ") {
			currentResult.Committer = strings.TrimSpace(lines[i][len("committer "):])
		}
	}

	if currentResult != nil {
		results = append(results, *currentResult)
	}

	return results, nil
}

func handleEmptyFile(repoPath, revision, file string) ([]BlameResult, error) {
	out, err := RunGitCommand(repoPath, "-C", repoPath, "log", "-1", "--format=\"%H%n%an\"", revision, "--", file)
	if err != nil {
		return []BlameResult{}, err
	}

	lines := strings.Split(out, "\n")

	return []BlameResult{
		{
			CommitHash: lines[0],
			Author:     strings.ReplaceAll(lines[1], `"`, ""),
		}}, nil
}

Пенис
Тромб 
Шляпа

