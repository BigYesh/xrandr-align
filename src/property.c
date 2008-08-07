/*
 * Copyright 2007 Peter Hutterer
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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <X11/Xatom.h>
#include <X11/extensions/XIproto.h>

#include "xinput.h"

static void
print_property(Display *dpy, XDevice* dev, Atom property)
{
    XIPropertyInfo      *propinfo;
    Atom                act_type;
    char                *name;
    int                 act_format;
    unsigned long       nitems, bytes_after;
    unsigned char       *data, *ptr;
    int                 j;

    propinfo = XQueryDeviceProperty(dpy, dev, property);
    if (!propinfo)
        return;

    name = XGetAtomName(dpy, property);
    printf("\t%s (%d):\t", name, property);

    if (XGetDeviceProperty(dpy, dev, property, 0, 1000, False, False,
                           AnyPropertyType, &act_type, &act_format,
                           &nitems, &bytes_after, &data) == Success)
    {
        ptr = data;
        printf("\t");

        for (j = 0; j < nitems; j++)
        {
            switch(act_type)
            {
                case XA_INTEGER:
                    switch(act_format)
                    {
                        case 8:
                            printf("%d", *((int8_t*)ptr));
                            break;
                        case 16:
                            printf("%d", *((int16_t*)ptr));
                            break;
                        case 32:
                            printf("%d", *((int32_t*)ptr));
                            break;
                    }
                    break;
                case XA_STRING:
                    printf("\t%s\n", ptr);
                    break;
                case XA_ATOM:
                    printf("\t%s\n", XGetAtomName(dpy, (Atom)(*ptr)));
                    break;
                default:
                    printf("\t\t... of unknown type %s\n",
                            XGetAtomName(dpy, act_type));
                    break;
            }

            ptr += act_format/8;

            if (j < nitems - 1)
                printf(", ");
            if (act_type == XA_STRING)
                break;
        }
        printf("\n");
        XFree(data);
    } else
        printf("\tFetch failure\n");

    if (propinfo->pending || propinfo->range || propinfo->immutable || propinfo->fromClient)
    {
        printf("\t\t%s%s%s%s", ((propinfo->pending) ? "[pending]" : ""),
                                  ((propinfo->range)   ? "[range]" : ""),
                                  ((propinfo->immutable) ? "[immutable]" : ""),
                                  ((propinfo->fromClient) ? "[client]" : ""));
        printf("\n");
    }

    if (propinfo->num_values)
    {
        long *values = propinfo->values;
        printf("\t\tvalid values: ");
        while(values && propinfo->num_values--)
            printf("%ld ", *values++);
        printf("\n");
    }

    XFree(propinfo);

}

int list_props(Display *dpy, int argc, char** argv, char* name, char *desc)
{
    XDeviceInfo *info;
    XDevice     *dev;
    int          i;
    int         nprops;
    Atom        *props;

    if (argc == 0)
    {
        fprintf(stderr, "Usage: xinput %s %s\n", name, desc);
        return EXIT_FAILURE;
    }

    for (i = 0; i < argc; i++)
    {
        info = find_device_info(dpy, argv[i], False);
        if (!info)
        {
            fprintf(stderr, "unable to find device %s\n", argv[i]);
            continue;
        }

        dev = XOpenDevice(dpy, info->id);
        if (!dev)
        {
            fprintf(stderr, "unable to open device '%s'\n", info->name);
            continue;
        }

        props = XListDeviceProperties(dpy, dev, &nprops);
        if (!nprops)
        {
            printf("Device '%s' does not report any properties.\n", info->name);
            continue;
        }

        printf("Device '%s':\n", info->name);
        while(nprops--)
        {
            print_property(dpy, dev, props[nprops]);
        }

        XFree(props);
        XCloseDevice(dpy, dev);
    }
    return EXIT_SUCCESS;
}

int
set_int_prop(Display *dpy, int argc, char** argv, char* n, char *desc)
{
    XDeviceInfo *info;
    XDevice     *dev;
    Atom         prop;
    char        *name;
    int          i;
    Bool         is_atom = True;
    char        *data;
    int          format, nelements =  0;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: xinput %s %s\n", n, desc);
        return EXIT_FAILURE;
    }

    info = find_device_info(dpy, argv[0], False);
    if (!info)
    {
        fprintf(stderr, "unable to find device %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    dev = XOpenDevice(dpy, info->id);
    if (!dev)
    {
        fprintf(stderr, "unable to open device %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    name = argv[1];

    for(i = 0; i < strlen(name); i++) {
	if (!isdigit(name[i])) {
            is_atom = False;
	    break;
	}
    }

    if (!is_atom)
        prop = XInternAtom(dpy, name, False);
    else
        prop = atoi(name);

    nelements = argc - 3;
    format    = atoi(argv[2]);
    if (format != 8 && format != 16 && format != 32)
    {
        fprintf(stderr, "Invalid format %d\n", format);
        return EXIT_FAILURE;
    }

    data = calloc(nelements, format/8);
    for (i = 0; i < nelements; i++)
    {
        switch(format)
        {
            case 8:
                *(((int8_t*)data) + i) = atoi(argv[3 + i]);
                break;
            case 16:
                *(((int16_t*)data) + i) = atoi(argv[3 + i]);
                break;
            case 32:
                *(((int32_t*)data) + i) = atoi(argv[3 + i]);
                break;
        }
    }

    XChangeDeviceProperty(dpy, dev, prop, XA_INTEGER, format, PropModeReplace,
                          (unsigned char*)data, nelements);

    free(data);
    XCloseDevice(dpy, dev);
    return EXIT_SUCCESS;
}


int watch_props(Display *dpy, int argc, char** argv, char* n, char *desc)
{
    XDevice     *dev;
    XDeviceInfo *info;
    XEvent      ev;
    XDevicePropertyNotifyEvent *dpev;
    char        *name;

    if (list_props(dpy, argc, argv, n, desc) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    info = find_device_info(dpy, argv[0], False);
    if (!info)
    {
        fprintf(stderr, "unable to find device %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    dev = XOpenDevice(dpy, info->id);
    if (!dev)
    {
        fprintf(stderr, "unable to open device '%s'\n", info->name);
        return EXIT_FAILURE;
    }

    XiSelectEvent(dpy, DefaultRootWindow(dpy), NULL,
                  XI_DevicePropertyNotifyMask);

    while(1)
    {
        XNextEvent(dpy, &ev);

        dpev = (XDevicePropertyNotifyEvent*)&ev;
        if (dpev->type != GenericEvent &&
            dpev->type != XI_DevicePropertyNotify)
            continue;
        name = XGetAtomName(dpy, dpev->atom);
        printf("Property '%s' changed.\n", name);
        print_property(dpy, dev, dpev->atom);
    }

    XCloseDevice(dpy, dev);
}
