/*
 * Copyright (c) 2026, Mauro Risonho de Paula Assumpção
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Mauro Risonho de Paula Assumpção
 * Date Created: 2026-02-14
 * Last Modified: 2026-02-14
 *
 * Project: FreeBSD Automotive Hardware Monitoring (HWMON)
 * Component: Kernel Driver Skeleton (hwmon_skeleton)
 */

#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <sys/rman.h>

/**
 * @file hwmon_skeleton.c
 * @brief Reference implementation for a HWMON driver utilizing the FreeBSD newbus architecture.
 *
 * This skeleton demonstrates the lifecycle management (probe, attach, detach) 
 * for a hardware monitoring device, suitable for automotive-grade sensors.
 */

struct hwmon_softc {
    device_t dev;
    struct resource *res;
};

static int
hwmon_probe(device_t dev)
{
    device_set_desc(dev, "High-Precision Automotive Hardware Monitor");
    return (BUS_PROBE_SPECIFIC);
}

static int
hwmon_attach(device_t dev)
{
    struct hwmon_softc *sc = device_get_softc(dev);
    sc->dev = dev;
    device_printf(dev, "[INIT] Attached HWMON Extension\n");
    return (0);
}

static int
hwmon_detach(device_t dev)
{
    device_printf(dev, "[SHUTDOWN] Detached HWMON Extension\n");
    return (0);
}

static device_method_t hwmon_methods[] = {
    DEVMETHOD(device_probe,  hwmon_probe),
    DEVMETHOD(device_attach, hwmon_attach),
    DEVMETHOD(device_detach, hwmon_detach),
    DEVMETHOD_END
};

static driver_t hwmon_driver = {
    "hwmon_poc",
    hwmon_methods,
    sizeof(struct hwmon_softc)
};

static devclass_t hwmon_devclass;

DRIVER_MODULE(hwmon_poc, nexus, hwmon_driver, hwmon_devclass, 0, 0);
MODULE_VERSION(hwmon_poc, 1);
