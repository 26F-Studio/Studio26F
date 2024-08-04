#include <format>

#include <drogon/drogon.h>

using namespace std;

int main() {
    LOG_INFO << format("{} v{} starting...", CMAKE_PROJECT_NAME, VERSION_STRING);
    drogon::app().loadConfigFile("config.json");
    drogon::app().run();
    return 0;
}
