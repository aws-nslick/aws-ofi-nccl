
#include <filesystem>
#include <string_view>
#include <range/v3/all.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>

namespace fs = std::filesystem;

struct sysfs_prober {
  struct device_directory {
    std::string_view path_str;
    fs::path path{fs::exists(path_str) ? fs::canonical(path_str) : path_str};

    device_directory(fs::path path) : path{fs::canonical(path)} {};

    /*
     * A valid "device directory" is one like:
     *  /sys/devices/pci0000:44/0000:44:00.0/0000:45:00.0/0000:46:01.4/0000:53:00.0
     *
     * Such a device directory could also be resolved through a symlink:
     *  /sys/module/nvidia/drivers/pci:nvidia/0000:53:00.0
     *
     */
    [[nodiscard]] bool valid() const {
      // Bad paths are not device directories.
      if (not fs::exists(path)) {
        return false;
      }

      // If it's not a directory, it's not a device directory.
      if (not fs::is_directory(path)) {
        return false;
      }

      // All device directories should start with "/sys/devices"
      if (not(path.string().rfind("/sys/devices/", 0) == 0)) {
        return false;
      }

      // All device directories should have a driver symlink.
      if (not fs::is_symlink((path / "driver"))) {
        return false;
      }

      return true;
    };
  };

  struct driver_directory {
    std::string_view path_str;
    fs::path path{fs::exists(path_str) ? fs::canonical(path_str) : path_str};

    [[nodiscard]] auto device_directories() const {
      auto const rng{fs::directory_iterator(path)};
      return rng | ranges::views::transform([](auto const& p) {
               return device_directory{p};
             }) |
          ranges::to_vector;
    };

    [[nodiscard]] auto devices() const {
      auto const dirs = device_directories();
      return dirs |
          ranges::views::filter([](auto const& d) { return d.valid(); }) |
          ranges::to_vector;
    }

    [[nodiscard]] auto valid() const {
      // It should exist.
      if (not fs::exists(path))
        return false;

      // It should be a directory.
      if (not fs::is_directory(path))
        return false;

      // A "module" symlink should exist in the directory.
      if (not(fs::exists(path / "module") and
              fs::is_symlink(path / "module"))) {
        return false;
      }

      // And it should contain at least one device.
      return ranges::any_of(
          device_directories(), [](auto d) { return d.valid(); });
    };
  };

  struct module_directory {
    static constexpr std::string_view base_path = "/sys/module/";
    std::string_view module_name;
    static fs::path makepath(std::string_view modname) {
      return fs::path(base_path) / modname;
    };

    fs::path path{
        fs::exists(makepath(module_name)) ? fs::canonical(makepath(module_name))
                                          : makepath(module_name)};

    [[nodiscard]] auto valid() const {
      // It should exist.
      if (not fs::exists(path))
        return false;

      // It should be a directory.
      if (not fs::is_directory(path))
        return false;

      // It should have a drivers directory.
      if (not(fs::exists(path / "drivers")))
        return false;

      // And it should contain at least one driver.
      auto const rng{fs::directory_iterator(path / "drivers")};
      return ranges::any_of(rng, [](auto const& p) {
        return driver_directory{p.path().c_str()}.valid();
      });
    };

    [[nodiscard]] auto drivers() const {
      if (valid()) {
        auto const rng{fs::directory_iterator(path / "drivers")};
        return rng | ranges::views::transform([](auto const& p) {
                 return driver_directory{p.path().c_str()};
               }) |
            ranges::views::filter([](auto const& p) { return p.valid(); }) |
            ranges::to_vector;
      }
      return std::vector<driver_directory>{};
    }

    [[nodiscard]] bool found() const {
      return ranges::any_of(drivers(), [](auto const& drv) {
        return not ranges::empty(drv.devices());
      });
    };
  };
};

static std::array modules = {
    sysfs_prober::module_directory{"nvidia"},
    sysfs_prober::module_directory{"neuron"}};

#include <iostream>

int main() {
  auto found_modules =
      ranges::views::filter(modules, [](auto mod) { return mod.found(); });
  if (found_modules.empty()) {
    std::cout << "found no accelerator\n";
    return 1;
  }
  std::cout << "found accelerator: " << ranges::front(found_modules).module_name
            << '\n';
  return 0;
}
