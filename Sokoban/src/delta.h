#ifndef DELTA_H
#define DELTA_H

#include "point.h"

class Delta {
public:
	virtual ~Delta();
	virtual void revert() = 0;
};


class DeltaFrame {
public:
	DeltaFrame();
	~DeltaFrame();
	void revert();
	void push(std::unique_ptr<Delta>);
	bool trivial();

	void reset_changed();
	bool changed();

private:
	std::vector<std::unique_ptr<Delta>> deltas_{};
	bool changed_ = false;
};


class UndoStack {
public:
	UndoStack(unsigned int max_depth);
	~UndoStack();
	void push(std::unique_ptr<DeltaFrame>);
	bool non_empty();
	void pop();
	void reset();

private:
	std::deque<std::unique_ptr<DeltaFrame>> frames_;
	unsigned int max_depth_;
	unsigned int size_;
};

#endif // DELTA_H
