#pragma once

#include <string>

class DrawEngine;
class DrawContext;

class PipeStage
{
public:
	PipeStage(const std::string &name, const DrawEngine& de);
	virtual void emit(void *data) = 0;
	virtual void finalize() = 0;

	// accessors
	PipeStage *getNextStage() const { return mNextStage; }
	const std::string & getName() const { return mName; }

	// mutators
	void setNextStage(PipeStage *stage) { mNextStage = stage; }

private:
	const DrawEngine& mDrawEngine;
	PipeStage *mNextStage;
	const std::string mName;
};