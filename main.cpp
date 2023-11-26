#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include "httplib.h"
#include "json.hpp"

#define PORT 3030

using json = nlohmann::json;

void observationTime(json &data) {
    data["observationTime"] = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

void cors(httplib::Response &res) {
    res.set_header("Access-Control-Allow-Origin", "http://localhost:3000");
    res.set_header("Access-Control-Allow-Methods", "GET");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

void add_value_route(httplib::Server &svr, const char *route, signed short (*value_function)()) {
    svr.Get(route, [value_function](const httplib::Request &req, httplib::Response &res) {
        json data;
        data["value"] = value_function();
        observationTime(data);
        cors(res);
        res.set_content(data.dump(), "application/json");
    });
}

void add_port_value_route(httplib::Server &svr, const char *route, int (*value_function)(int)) {
    svr.Get(route, [value_function](const httplib::Request &req, httplib::Response &res) {
        try {
            auto port_str = req.get_param_value("port");
            if (port_str.empty())
                throw std::runtime_error("Missing port");

            int port = std::stoi(port_str);
            json data;
            data["value"] = value_function(port);
            observationTime(data);
            cors(res);
            res.set_content(data.dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("Bad Request: Invalid or missing 'port' query parameter", "text/plain");
        }
    });
}

int main() {
    using namespace httplib;
    void *lib_handle = dlopen("/usr/lib/libkipr.so", RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Error loading libkipr.so: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    Server svr;

    add_value_route(svr, "/gyro_x", (signed short (*)()) dlsym(lib_handle, "gyro_x"));
    add_value_route(svr, "/gyro_y", (signed short (*)()) dlsym(lib_handle, "gyro_y"));
    add_value_route(svr, "/gyro_z", (signed short (*)()) dlsym(lib_handle, "gyro_z"));

    add_value_route(svr, "/magneto_x", (signed short (*)()) dlsym(lib_handle, "magneto_x"));
    add_value_route(svr, "/magneto_y", (signed short (*)()) dlsym(lib_handle, "magneto_y"));
    add_value_route(svr, "/magneto_z", (signed short (*)()) dlsym(lib_handle, "magneto_z"));

    add_value_route(svr, "/accel_x", (signed short (*)()) dlsym(lib_handle, "accel_x"));
    add_value_route(svr, "/accel_y", (signed short (*)()) dlsym(lib_handle, "accel_y"));
    add_value_route(svr, "/accel_z", (signed short (*)()) dlsym(lib_handle, "accel_z"));

    add_port_value_route(svr, "/digital", (int (*)(int)) dlsym(lib_handle, "digital"));
    add_port_value_route(svr, "/analog", (int (*)(int)) dlsym(lib_handle, "analog"));

    svr.listen("0.0.0.0", PORT);
    dlclose(lib_handle);
}