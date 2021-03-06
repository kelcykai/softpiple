#include "EGLDisplayX11.h"

#include <fcntl.h>
#include <unistd.h>

#include <X11/Xlib-xcb.h>
#include <xcb/dri2.h>

#include "EGLContextX11.h"
#include "EGLSurfaceX11.h"
#include "common/glsp_defs.h"
#include "khronos/EGL/egl.h"


NS_OPEN_GLSP_EGL()

EGLDisplayX11::EGLDisplayX11(void *nativeDpy, EGLenum platformType):
	EGLDisplayBase(nativeDpy, platformType),
	mXCBConn(NULL),
	mXCBScreen(NULL),
	mDrmFd(-1),
	mDri2Major(0),
	mDri2Minor(0)
{
}

EGLDisplayX11::~EGLDisplayX11()
{
	if(mDrmFd)
		::close(mDrmFd);

	if(mBufMgr)
		::drm_intel_bufmgr_destroy(mBufMgr);
}

bool EGLDisplayX11::initDisplay()
{
	if(!EGLDisplayBase::initDisplay())
		goto err_out;
	
	{
		// The native display should be a Display opened through Xlib,
		// which was passed in when eglGetDisplay().
		Display *dpy = static_cast<Display *>(getNativeDisplay());
		xcb_connection_t *c = NULL;
		
		if(dpy)
		{
			c = ::XGetXCBConnection(dpy);
		}

		if(!c || ::xcb_connection_has_error(c))
			goto err_out;

		mXCBScreen = ::xcb_setup_roots_iterator(xcb_get_setup(c)).data;
		mXCBConn   = c;
	}

	::xcb_dri2_connect_cookie_t connect_cookie;
	::xcb_dri2_connect_reply_t *connect;
	connect_cookie = ::xcb_dri2_connect_unchecked(mXCBConn,
												mXCBScreen->root,
												XCB_DRI2_DRIVER_TYPE_DRI);

	::xcb_dri2_query_version_cookie_t dri2_query_cookie;
	::xcb_dri2_query_version_reply_t *dri2_query;
	::xcb_generic_error_t *error;
	dri2_query_cookie = ::xcb_dri2_query_version(mXCBConn,
											   XCB_DRI2_MAJOR_VERSION,
											   XCB_DRI2_MINOR_VERSION);
	dri2_query = ::xcb_dri2_query_version_reply(mXCBConn,
											  dri2_query_cookie,
											  &error);
	if(dri2_query == NULL || error != NULL)
		goto err_conn;

	mDri2Major = dri2_query->major_version;
	mDri2Minor = dri2_query->minor_version;
	::free(dri2_query);

	connect = ::xcb_dri2_connect_reply (mXCBConn, connect_cookie, NULL);

	if (connect == NULL ||
		connect->driver_name_length + connect->device_name_length == 0)
		goto err_version;

	char *driver_name, *device_name;
	driver_name = ::xcb_dri2_connect_driver_name(connect);
	device_name = ::xcb_dri2_connect_device_name(connect);
	mDriverName = driver_name;
	mDeviceName = device_name;

	// use mDriverName & mDeviceName
	//mDrmFd = ::drmOpen("i915", NULL);
	mDrmFd = ::open(device_name, O_RDWR | O_CLOEXEC);
	::free(connect);


	if(mDrmFd < 0)
		goto err_fd;

	::drm_magic_t magic;
	if (::drmGetMagic(mDrmFd, &magic))
		goto err_fd;

	::xcb_dri2_authenticate_reply_t *authenticate;
	::xcb_dri2_authenticate_cookie_t authenticate_cookie;
	authenticate_cookie =
		::xcb_dri2_authenticate_unchecked(mXCBConn, mXCBScreen->root, magic);
	authenticate =
		::xcb_dri2_authenticate_reply(mXCBConn, authenticate_cookie, NULL);

	if(authenticate == NULL || !authenticate->authenticated)
		goto err_fd;

	::free(authenticate);

	mBufMgr = ::drm_intel_bufmgr_gem_init(mDrmFd, batch_size_);

	return true;

err_fd:
	if(mDrmFd >= 0)
	{
		drmClose(mDrmFd);
		mDrmFd = -1;
	}

	mDeviceName.clear();
	mDriverName.clear();

err_version:
	mDri2Minor = 0;
	mDri2Major = 0;

err_conn:
	mXCBScreen = NULL;
	mXCBConn   = NULL;

err_out:
	return false;
}

EGLSurfaceBase* EGLDisplayX11::createWindowSurface(
		EGLConfigBase *config,
		EGLNativeWindowType win,
		const EGLint *attrib_list)
{
	// TODO: impl
	GLSP_UNREFERENCED_PARAM(config);
	GLSP_UNREFERENCED_PARAM(attrib_list);

	EGLSurfaceBase *pSur = new EGLSurfaceX11(*this, EGL_WINDOW_BIT);

	pSur->initSurface(this, win);

	attachResource(pSur);

	return pSur;
}

EGLContextBase* EGLDisplayX11::createContext(
			EGLConfigBase *config,
			EGLContextBase *share_context,
			const EGLint *attrib_list)
{
	GLSP_UNREFERENCED_PARAM(config);
	GLSP_UNREFERENCED_PARAM(share_context);
	GLSP_UNREFERENCED_PARAM(attrib_list);

	EGLContextBase *pCtx = new EGLContextX11(*this);

	pCtx->initContext();

	attachResource(pCtx);

	return pCtx;
}

EGLBoolean EGLDisplayX11::makeCurrent(
			EGLSurfaceBase *draw,
			EGLSurfaceBase *read,
			EGLContextBase *ctx)
{
	bindContext(ctx);
	ctx->bindSurface(read, draw);

	return EGL_TRUE;
}


NS_CLOSE_GLSP_EGL()
