#ifndef SERVER_APPLICATION_H_
#define SERVER_APPLICATION_H_

#include <atomic>
#include <string>

class ServerApplication {
public:
    ServerApplication(const std::string &);

    void run();

private:
    bool pidFileOpen() const;
    void pidFileClose() const;
    sigset_t setupSignalHandlers() const;
    void keepProcessingSignals(sigset_t *);

    // bool onInit(const std::string& config) override;
    // void onStart() override;
    // void onStop() override;

    std::atomic_bool mKeepGoing{true};
    std::string mPidFilename;
    std::string mLogFilename;
    std::string mLoggername;
};

#endif // SERVER_APPLICATION_H_