#include <drogon/drogon.h>

int main() {
    LOG_INFO << std::format("{} v{} starting...", CMAKE_PROJECT_NAME, VERSION_STRING);
    drogon::app().loadConfigFile("config.json");
    drogon::app().run();
    return 0;
}
