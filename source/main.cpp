#ifndef APP_VERSION
#define APP_VERSION "1.0.0"
#endif

#define TESLA_INIT_IMPL
#include <minIni.h>
#include <tesla.hpp>

#include <string>
#include <vector>

#include "ntp-client.hpp"
#include "tesla-ext.hpp"

TimeServiceType __nx_time_service_type = TimeServiceType_System;

const char* iniLocations[] = {
    "/config/quickntp.ini",
    "/config/quickntp/config.ini",
    "/switch/.overlays/quickntp.ini",
};
const char* iniSection = "Servers";

const char* defaultServerAddress = "ntp.ntsc.ac.cn ";
const char* defaultServerName = "ChinaDefault";

class NtpGui : public tsl::Gui {
private:
    std::string Message = "";
    int currentServer = 0;
    bool blockFlag = false;
    std::vector<std::string> serverAddresses;
    std::vector<std::string> serverNames;

    std::string getCurrentServerAddress() {
        return serverAddresses[currentServer];
    }

    bool setNetworkSystemClock(time_t time) {
        Result rs = timeSetCurrentTime(TimeType_NetworkSystemClock, (uint64_t)time);
        if (R_FAILED(rs)) {
            return false;
        }
        return true;
    }

    void setTime() {
        std::string srv = getCurrentServerAddress();
        NTPClient* client = new NTPClient(srv.c_str());

        try {
            time_t ntpTime = client->getTime();

            if (setNetworkSystemClock(ntpTime)) {
                Message = "已同步 " + srv;
            } else {
                Message = "无法设置网络时钟。";
            }
        } catch (const NtpException& e) {
            Message = "错误: " + std::string(e.what());
        }

        delete client;
    }

    void
    setNetworkTimeAsUser() {
        time_t userTime, netTime;

        Result rs = timeGetCurrentTime(TimeType_UserSystemClock, (u64*)&userTime);
        if (R_FAILED(rs)) {
            Message = "获取用户时间" + std::to_string(rs);
            return;
        }

        std::string usr = "用户时间!";
        std::string gr8 = "";
        rs = timeGetCurrentTime(TimeType_NetworkSystemClock, (u64*)&netTime);
        if (!R_FAILED(rs) && netTime < userTime) {
            gr8 = " Great Scott!";
        }

        if (setNetworkSystemClock(userTime)) {
            Message = usr.append(gr8);
        } else {
            Message = "无法设置网络时钟。";
        }
    }

    void getOffset() {
        time_t currentTime;
        Result rs = timeGetCurrentTime(TimeType_NetworkSystemClock, (u64*)&currentTime);
        if (R_FAILED(rs)) {
            Message = "获取网络时间 " + std::to_string(rs);
            return;
        }

        std::string srv = getCurrentServerAddress();
        NTPClient* client = new NTPClient(srv.c_str());

        try {
            time_t ntpTimeOffset = client->getTimeOffset(currentTime);
            Message = "偏移量: " + std::to_string(ntpTimeOffset) + "s";
        } catch (const NtpException& e) {
            Message = "错误: " + std::string(e.what());
        }

        delete client;
    }

    bool operationBlock(std::function<void()> fn) {
        if (!blockFlag) {
            blockFlag = true;
            Message = "";
            fn(); // TODO: Call async and set blockFlag to false
            blockFlag = false;
        }
        return !blockFlag;
    }

    std::function<std::function<bool(u64 keys)>(int key)> syncListener = [this](int key) {
        return [=, this](u64 keys) {
            if (keys & key) {
                return operationBlock([&]() {
                    setTime();
                });
            }
            return false;
        };
    };

    std::function<std::function<bool(u64 keys)>(int key)> offsetListener = [this](int key) {
        return [=, this](u64 keys) {
            if (keys & key) {
                return operationBlock([&]() {
                    getOffset();
                });
            }
            return false;
        };
    };

public:
    NtpGui() {
        char key[INI_BUFFERSIZE];
        char value[INI_BUFFERSIZE];

        const char* iniFile = iniLocations[0];
        for (const char* loc : iniLocations) {
            INI_FILETYPE fp;
            if (ini_openread(loc, &fp)) {
                iniFile = loc;
                ini_close(&fp);
                break;
            }
        }

        int idx = 0;
        while (ini_getkey(iniSection, idx++, key, INI_BUFFERSIZE, iniFile) > 0) {
            ini_gets(iniSection, key, "", value, INI_BUFFERSIZE, iniFile);
            serverAddresses.push_back(value);

            std::string keyStr = key;
            std::replace(keyStr.begin(), keyStr.end(), '_', ' ');
            serverNames.push_back(keyStr);
        }

        if (serverNames.empty() || serverAddresses.empty()) {
            serverNames.push_back(defaultServerName);
            serverAddresses.push_back(defaultServerAddress);
        }
    }

    virtual tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::CustomOverlayFrame("QuickNTP", std::string("by NedEX 翻译:Cirrest - v") + APP_VERSION);

        auto list = new tsl::elm::List();

        list->setClickListener([this](u64 keys) {
            if (keys & (HidNpadButton_AnyUp | HidNpadButton_AnyDown | HidNpadButton_AnyLeft | HidNpadButton_AnyRight)) {
                Message = "";
                return true;
            }
            return false;
        });

        list->addItem(new tsl::elm::CategoryHeader("选择服务器   |   \uE0E0  同步   |   \uE0E3  偏移量"));

        auto* trackbar = new tsl::elm::NamedStepTrackBarVector("\uE017", serverNames);
        trackbar->setValueChangedListener([this](u8 val) {
            currentServer = val;
            Message = "";
        });
        trackbar->setClickListener([this](u8 val) {
            return syncListener(HidNpadButton_A)(val) || offsetListener(HidNpadButton_Y)(val);
        });
        list->addItem(trackbar);

        auto* syncTimeItem = new tsl::elm::ListItem("同步时间");
        syncTimeItem->setClickListener(syncListener(HidNpadButton_A));
        list->addItem(syncTimeItem);

        list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
                          renderer->drawString("将时间与选定的服务器同步。", false, x + 20, y + 20, 15, renderer->a(tsl::style::color::ColorDescription));
                      }),
                      50);

        auto* getOffsetItem = new tsl::elm::ListItem("获取偏移量");
        getOffsetItem->setClickListener(offsetListener(HidNpadButton_A));
        list->addItem(getOffsetItem);

        list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
                          renderer->drawString("获取与所选服务器的秒数偏移量。\n\n\uE016  各位吴彦祖刘亦菲们，±3秒的误差是正常不影响使用的。", false, x + 20, y + 20, 15, renderer->a(tsl::style::color::ColorDescription));
                      }),
                      70);

        auto* setToInternalItem = new tsl::elm::ListItem("设置用户时间");
        setToInternalItem->setClickListener([this](u64 keys) {
            if (keys & HidNpadButton_A) {
                return operationBlock([&]() {
                    setNetworkTimeAsUser();
                });
            }
            return false;
        });
        list->addItem(setToInternalItem);

        list->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
                          renderer->drawString("将网络时间设置为用户设置的时间。", false, x + 20, y + 20, 15, renderer->a(tsl::style::color::ColorDescription));
                      }),
                      50);

        list->addItem(new tsl::elm::CustomDrawerUnscissored([&message = Message](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
            if (!message.empty()) {
                renderer->drawString(message.c_str(), false, x + 5, tsl::cfg::FramebufferHeight - 100, 20, renderer->a(tsl::style::color::ColorText));
            }
        }));

        frame->setContent(list);
        return frame;
    }
};

class NtpOverlay : public tsl::Overlay {
public:
    virtual void initServices() override {
        constexpr SocketInitConfig socketInitConfig = {
            // TCP buffers
            .tcp_tx_buf_size = 16 * 1024,         // 16 KB default
            .tcp_rx_buf_size = 16 * 1024 * 2,     // 16 KB default
            .tcp_tx_buf_max_size = 64 * 1024,     // 64 KB default max
            .tcp_rx_buf_max_size = 64 * 1024 * 2, // 64 KB default max

            // UDP buffers
            .udp_tx_buf_size = 512, // 512 B default
            .udp_rx_buf_size = 512, // 512 B default

            // Socket buffer efficiency
            .sb_efficiency = 1,                     // 0 = default, balanced memory vs CPU
                                                    // 1 = prioritize memory efficiency (smaller internal allocations)
            .bsd_service_type = BsdServiceType_Auto // Auto-select service
        };
        ASSERT_FATAL(socketInitialize(&socketInitConfig));
        ASSERT_FATAL(nifmInitialize(NifmServiceType_User));
        ASSERT_FATAL(timeInitialize());
        ASSERT_FATAL(smInitialize()); // Needed
    }

    virtual void exitServices() override {
        socketExit();
        nifmExit();
        timeExit();
        smExit();
    }

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<NtpGui>();
    }
};

int main(int argc, char** argv) {
    return tsl::loop<NtpOverlay>(argc, argv);
}
