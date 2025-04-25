package domain

import "time"

type FeedingSchedule struct {
	ID       string
	AnimalID string
	Time     time.Time
	FoodType string
	IsDone   bool
}

func (f *FeedingSchedule) MarkCompleted() {
	f.IsDone = true
}

func (f *FeedingSchedule) Reschedule(newTime time.Time) {
	f.Time = newTime
	f.IsDone = false
}
