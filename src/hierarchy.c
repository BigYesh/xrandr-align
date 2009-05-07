/*
 * Copyright 2007 Peter Hutterer <peter@cs.unisa.edu.au>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the author shall
 * not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization
 * from the author.
 *
 */

#include "xinput.h"
#include <string.h>

#define Error(error, ...) \
{ \
    fprintf(stderr, __VA_ARGS__); \
    return error;\
}
/**
 * Create a new master device. Name must be supplied, other values are
 * optional.
 */
int
create_master(Display* dpy, int argc, char** argv, char* name, char *desc)
{
    XICreateMasterInfo c;

    if (argc == 0)
    {
        fprintf(stderr, "Usage: xinput %s %s\n", name, desc);
        return EXIT_FAILURE;
    }

    c.type = CH_CreateMasterDevice;
    c.name = argv[0];
    c.sendCore = (argc >= 2) ? atoi(argv[1]) : 1;
    c.enable = (argc >= 3) ? atoi(argv[2]) : 1;

    return XIChangeDeviceHierarchy(dpy, (XIAnyHierarchyChangeInfo*)&c, 1);
}

/**
 * Remove a master device.
 * By default, all attached devices are set to Floating, unless parameters are
 * given.
 */
int
remove_master(Display* dpy, int argc, char** argv, char *name, char *desc)
{
    XIRemoveMasterInfo r;
    XIDeviceInfo *info;
    int ret;

    if (argc == 0)
    {
        fprintf(stderr, "usage: xinput %s %s\n", name, desc);
        return EXIT_FAILURE;
    }

    info = xi2_find_device_info(dpy, argv[0]);

    if (!info) {
	fprintf(stderr, "unable to find device %s\n", argv[0]);
	return EXIT_FAILURE;
    }

    r.type = CH_RemoveMasterDevice;
    r.device = info->deviceid;
    if (argc >= 2)
    {
        if (!strcmp(argv[1], "Floating"))
            r.returnMode = Floating;
        else if (!strcmp(argv[1], "AttachToMaster"))
            r.returnMode = AttachToMaster;
        else
            Error(BadValue, "Invalid returnMode.\n");
    } else
        r.returnMode = Floating;

    if (r.returnMode == AttachToMaster)
    {
        r.returnPointer = atoi(argv[2]);
        r.returnKeyboard = atoi(argv[3]);
    }

    ret = XIChangeDeviceHierarchy(dpy, (XIAnyHierarchyChangeInfo*)&r, 1);
    return ret;
}

/**
 * Swap a device from one master to another.
 */
int
change_attachment(Display* dpy, int argc, char** argv, char *name, char* desc)
{
    XIDeviceInfo *sd_info, *md_info;
    XIAttachSlaveInfo c;
    int ret;

    if (argc < 2)
    {
        fprintf(stderr, "usage: xinput %s %s\n", name, desc);
        return EXIT_FAILURE;
    }

    sd_info = xi2_find_device_info(dpy, argv[0]);
    md_info= xi2_find_device_info(dpy, argv[1]);

    if (!sd_info) {
	fprintf(stderr, "unable to find device %s\n", argv[0]);
	return EXIT_FAILURE;
    }

    if (!md_info) {
	fprintf(stderr, "unable to find device %s\n", argv[1]);
	return EXIT_FAILURE;
    }

    c.type = CH_AttachSlave;
    c.device = sd_info->deviceid;
    c.newMaster = md_info->deviceid;

    ret = XIChangeDeviceHierarchy(dpy, (XIAnyHierarchyChangeInfo*)&c, 1);
    return ret;
}

/**
 * Set a device floating.
 */
int
float_device(Display* dpy, int argc, char** argv, char* name, char* desc)
{
    XIDeviceInfo *info;
    XIDetachSlaveInfo c;
    int ret;

    if (argc < 1)
    {
        fprintf(stderr, "usage: xinput %s %s\n", name, desc);
        return EXIT_FAILURE;
    }

    info = xi2_find_device_info(dpy, argv[0]);

    if (!info) {
	fprintf(stderr, "unable to find device %s\n", argv[0]);
	return EXIT_FAILURE;
    }

    c.type = CH_DetachSlave;
    c.device = info->deviceid;

    ret = XIChangeDeviceHierarchy(dpy, (XIAnyHierarchyChangeInfo*)&c, 1);
    return ret;
}


