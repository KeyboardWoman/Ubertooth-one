/*
    This file is part of Kismet

    Kismet is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kismet is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Kismet; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __DEVICE_VIEW_H__
#define __DEVICE_VIEW_H__

#include "config.h"

#include <functional>

#include "kis_mutex.h"
#include "uuid.h"
#include "trackedelement.h"
#include "trackedcomponent.h"
#include "devicetracker_component.h"
#include "devicetracker_view_workers.h"

// Common view holder mechanism which handles view endpoints, view filtering, and so on.
//
// Views are optimized for maintinaing independent, sorted lists of devices.  For a view to work,
// it requires assisting code in the appropriate areas (such as adding a SSID to a dot11 device).
//
// Views are best suited to long-term alternate representations of data, such as 'all access points',
// 'all devices of a given phy type', and so on.  The vector-backed system is not well optimized
// for frequent eviction of devices from the view.
//
// Views live under the devices tree in:
// /devices/view/[view id]/...
//
// Main device sorting/filtering/datatables view lives under:
// /devices/view/[view id]/devices.json

class kis_tracked_device;
class DevicetrackerView;

class DevicetrackerView : public tracker_component {
public:
    // The new device callback is called whenever a new device is created by the devicetracker;
    // it's also called for every device when a new view is created, to perform the initial 
    // population

    // The updated device callback is called whenever a change event occurs.  Change events
    // are triggered by specific code, make sure you've integrated a change trigger for
    // the filtering you're performing.
    // Returning 'false' removes the device from the list.  This is expensive, so 
    // removing devices from filtering should be a relatively rare event
    
    using new_device_cb = std::function<bool (std::shared_ptr<kis_tracked_device_base>)>;
    using updated_device_cb = std::function<bool (std::shared_ptr<kis_tracked_device_base>)>;

    DevicetrackerView(const std::string& in_id, const std::string& in_description,
            new_device_cb in_new_cb, updated_device_cb in_upd_cb);

    virtual ~DevicetrackerView() {
        local_locker l(&mutex);
    }

    // Protect proxies w/ mutex
    __ProxyGet(view_id, std::string, std::string, view_id);
    __ProxyGet(view_description, std::string, std::string, view_description);

    virtual void pre_serialize() override {
        local_eol_shared_locker lock(&mutex);
        list_sz->set(device_list->size());
    }

    virtual void post_serialize() override {
        local_shared_unlocker lock(&mutex);
    }

    // Do work on the base list of all devices in this view; this makes an immutable copy
    // before perforing work
    virtual std::shared_ptr<TrackerElementVector> doDeviceWork(DevicetrackerViewWorker& worker);

    // Do work on a specific vector; this does NOT make an immutable copy of the vector.  You
    // must not call this on a vector which can be altered in another thread.
    virtual std::shared_ptr<TrackerElementVector> doDeviceWork(DevicetrackerViewWorker& worker,
            std::shared_ptr<TrackerElementVector> vec);

    // Called when a device undergoes a change that might make it eligible for inclusion
    // into a view; Integration with view filtering needs to be added to other locations
    // to activate this.
    virtual void updateDevice(std::shared_ptr<kis_tracked_device_base> device);

protected:
    virtual void register_fields() override {
        tracker_component::register_fields();

        RegisterField("kismet.devices.view.id", "View ID/Endpoint", &view_id);
        RegisterField("kismet.devices.view.description", "List description", &view_description);
        RegisterField("kismet.devices.view.size", "Number of device in list", &list_sz);

        // We don't register device_list as a field because we never want to dump it 
        // un-processed; use the view APIs for managing that
    }

    kis_recursive_timed_mutex mutex;

    std::shared_ptr<TrackerElementString> view_id;
    std::shared_ptr<TrackerElementUUID> view_uuid;
    std::shared_ptr<TrackerElementString> view_description;
    std::shared_ptr<TrackerElementUInt32> list_sz;

    new_device_cb new_cb;
    updated_device_cb update_cb;

    // Main vector of devices
    std::shared_ptr<TrackerElementVector> device_list;
    // Map of device presence in our list for fast referece during updates
    std::map<device_key, bool> device_presence_map;

    // Complex endpoint
    std::shared_ptr<Kis_Net_Httpd_Simple_Post_Endpoint> device_endp;

    // Simpler time-based endpoints
    std::shared_ptr<Kis_Net_Httpd_Path_Tracked_Endpoint> time_endp;

    // Complex post endp handler
    unsigned int device_endpoint_handler(std::ostream& stream, const std::string& uri, SharedStructured structured,
            std::map<std::string, std::shared_ptr<std::stringstream>>& postvars);

    // Time endp handler
    bool device_time_endpoint_path(const std::vector<std::string>& path);
    std::shared_ptr<TrackerElement> device_time_endpoint(const std::vector<std::string>& path);

    // Devicetracker has direct access to protected methods for new devices and purging devices,
    // nobody else should be calling those
    friend class Devicetracker;

    // Called when a device is created; this should only be called by devicetracker itself.
    virtual void newDevice(std::shared_ptr<kis_tracked_device_base> device);

    // Remove a device from any views; this is called when the devicetracker times out a 
    // device record.
    virtual void removeDevice(std::shared_ptr<kis_tracked_device_base> device);

};

#endif

