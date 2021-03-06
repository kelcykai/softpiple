#include "ScreenMapper.h"
#include "DataFlow.h"
#include "DrawEngine.h"
#include "GLContext.h"


NS_OPEN_GLSP_OGL()

using glm::vec4;

ScreenMapper::ScreenMapper():
	PipeStage("Viewport Transform", DrawEngine::getDrawEngine())
{
}

void ScreenMapper::emit(void *data)
{
	Batch *bat = static_cast<Batch *>(data);

	viewportTransform(bat);

	getNextStage()->emit(bat);
}

void ScreenMapper::viewportTransform(Batch *bat)
{
	GLContext   *gc = bat->mDC->gc;

	int xCenter = gc->mState.mViewport.xCenter;
	int yCenter = gc->mState.mViewport.yCenter;
	int xScale  = gc->mState.mViewport.xScale;
	int yScale  = gc->mState.mViewport.yScale;

	Primlist &pl = bat->mPrims;
	Primlist::iterator it = pl.begin();

	while(it != pl.end())
	{
		for(size_t i = 0; i < 3; ++i)
		{
			vec4 &pos = it->mVert[i].position();
			pos.x = xCenter + pos.x * xScale;
			pos.y = yCenter + pos.y * yScale;
			pos.z = (pos.z + 1) * 0.5f;
		}

		const vec4 &pos0 = it->mVert[0].position();
		const vec4 &pos1 = it->mVert[1].position();
		const vec4 &pos2 = it->mVert[2].position();

		float ex = pos1.x - pos0.x;
		float ey = pos1.y - pos0.y;
		float fx = pos2.x - pos0.x;
		float fy = pos2.y - pos0.y;
		float area = ex * fy - ey * fx;

		// Discard the triangles whose area are less than 1
		if(abs(area) <= 1.0f)
		{
			it = pl.erase(it);
		}
		else
		{
			it->mAreaReciprocal = 1.0f / area;
			it++;
		}
	}
}

void ScreenMapper::finalize()
{
}

NS_CLOSE_GLSP_OGL()
