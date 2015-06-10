#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"


NS_OPEN_GLSP_OGL()

using namespace std;

/* The wrapper for the whole rasterizer stages.
 * Also responsible for resource life management.
 */
class RasterizerWrapper: public PipeStage
{
	friend class DrawEngine;

public:
	RasterizerWrapper();
	~RasterizerWrapper();

	void SetupPipeStages();
	virtual void emit(void *data);
	virtual void finalize();

private:
	PipeStage* getFirstStage() const { return mFirstStage; }
	void setFirstStage(PipeStage *stage) { mFirstStage = stage; }

	PipeStage *mFirstStage;

	PipeStage *mpRasterizer; // scan conversion
	PipeStage *mpInterpolate;
	PipeStage *mpFS;
	PipeStage *mpOwnershipTest;
	PipeStage *mpScissorTest;
	PipeStage *mpAlphaTest;
	PipeStage *mpStencilTest;
	PipeStage *mpDepthTest;
	PipeStage *mpBlender;
	PipeStage *mpDither;
};

class Rasterizer: public PipeStage
{
public:
	Rasterizer();
	virtual ~Rasterizer() = default;

	virtual void emit(void *data);
	virtual void finalize();

	class Gradience;
	class fs_in_out;

protected:
	virtual void onRasterizing(Batch *bat) = 0;
};


/* Optimized incremental interpolate */

class Interpolater: public PipeStage
{
public:
	Interpolater() = default;
	virtual ~Interpolater() = default;

	// PipeStage interfaces
	virtual void emit(void *data);
	virtual void finalize();

	// Should be invoked before onInterpolating
	virtual void CalculateRadiences(Primitive &prim) = 0;

protected:
	virtual void onInterpolating(const fsInput &in,
								 const fsInput &gradX;
								 const fsInput &gradY;
								 float stepx, float stepy,
								 fsInput &out) = 0;
};


NS_CLOSE_GLSP_OGL()