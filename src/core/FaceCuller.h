#pragma once

#include "PipeStage.h"

class Batch;

class FaceCuller: public PipeStage
{
public:
	enum orient_t
	{
		CCW = 0,
		CW = 1
	};

	enum face_t
	{
		BACK = 0x1,
		FRONT = 0x2,
		FRONT_AND_BACK = 0x3
	};

	FaceCuller();
	virtual void emit(void *data);
	virtual void finalize();

	void culling(Batch *bat);
	bool CullEnabled() const { return mCullEnabled; }
	void setCullEnabled(bool enable) { mCullEnabled = enable; }

private:
	orient_t mOrient;
	face_t mCullFace;
	bool mCullEnabled;
};