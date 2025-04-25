package domain

import (
	"errors"
	"time"

	"github.com/google/uuid"
)

type HealthStatus string
type Gender string

const (
	Healthy HealthStatus = "healthy"
	Sick    HealthStatus = "sick"

	Male   Gender = "male"
	Female Gender = "female"
)

type Animal struct {
	ID               string
	Name             string
	Species          string
	BirthDate        time.Time
	Gender           Gender
	FavoriteFood     string
	Health           HealthStatus
	CurrentEnclosure string
}

func NewAnimal(name, species string, birthDate time.Time, gender Gender, favoriteFood string) (*Animal, error) {
	if name == "" || species == "" {
		return nil, errors.New("name and species are required")
	}

	animal := &Animal{
		ID:           uuid.New().String(),
		Name:         name,
		Species:      species,
		BirthDate:    birthDate,
		Gender:       gender,
		FavoriteFood: favoriteFood,
		Health:       Healthy,
	}
	return animal, nil
}

func (a *Animal) Feed() {
}

func (a *Animal) Heal() {
	a.Health = Healthy
}

func (a *Animal) MoveToEnclosure(enclosureID string) {
	a.CurrentEnclosure = enclosureID
}
