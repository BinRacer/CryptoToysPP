#include <wx/wx.h>
#include <wx/webview.h>
#include <wx/mstream.h>
#include <wx/datetime.h>
#include <wx/filesys.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <ctime>
#include <functional>
#include <unordered_map>
#include <fstream>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/strconv.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "resources.h"

using json = nlohmann::json;

// 修复后的协议处理器
class AppSchemeHandler : public wxWebViewHandler {
public:
    AppSchemeHandler() : wxWebViewHandler("app") {}

    virtual wxFSFile* GetFile(const wxString& uri) {
        // 解析请求路径: "app://path/to/file"
        wxString path = uri.AfterFirst(':');

        // 移除协议后的所有前导斜杠
        while (path.StartsWith("/")) {
            path = path.Mid(1);
        }

        // 添加前导斜杠作为统一路径前缀
        path = "/" + path;
        path.Replace("\\", "/", true); // 统一路径格式

        const std::string key = path.ToStdString();

        spdlog::info("资源请求: {}", key);

        // 查找资源
        auto it = RESOURCE_MAP.find(key);
        if (it == RESOURCE_MAP.end()) {
            spdlog::warn("资源未找到: {}", key);
            return nullptr;
        }

        // 设置MIME类型
        wxString mime = "text/plain";
        if (path.EndsWith(".html")) mime = "text/html";
        else if (path.EndsWith(".js")) mime = "application/javascript";
        else if (path.EndsWith(".css")) mime = "text/css";
        else if (path.EndsWith(".png")) mime = "image/png";
        else if (path.EndsWith(".jpg")) mime = "image/jpeg";
        else if (path.EndsWith(".json")) mime = "application/json";
        else if (path.EndsWith(".ico")) mime = "image/x-icon";

        // 创建内存数据流
        const auto& data = it->second;
        wxMemoryInputStream* stream = new wxMemoryInputStream(
            data.data(), data.size()
        );

        // 返回文件对象
        return new wxFSFile(
            stream,        // 输入流
            uri,           // 原始URI
            mime,          // MIME类型
            wxEmptyString, // 锚点
            wxDateTime::Now() // 修改时间
        );
    }
};

class ApiHandler {
public:
    using ApiFunction = std::function<json(const json&)>;

    ApiHandler() {
        registerApi("/api/getUser", [this](const json&) { return handleGetUser(); });
        registerApi("/api/getTime", [this](const json&) { return handleGetTime(); });
        registerApi("/api/calculate", [this](const json& data) { return handleCalculate(data); });
    }

    void registerApi(const std::string& path, ApiFunction handler) {
        apiMap[path] = handler;
    }

    std::string handleRequest(const json& request) {
        json response;
        try {
            std::string path = request.at("path").get<std::string>();
            json data = request.value("data", json::object());

            auto it = apiMap.find(path);
            if (it != apiMap.end()) {
                response = it->second(data);
            } else {
                response = {
                    {"code", 404},
                    {"message", "Endpoint not found: " + path},
                    {"data", json::object()}
                };
            }
        }
        catch (const std::exception& e) {
            response = {
                {"code", 400},
                {"message", "Bad request: " + std::string(e.what())},
                {"data", json::object()}
            };
        }

        return response.dump();
    }

private:
    json handleGetUser() {
        return {
            {"code", 200},
            {"message", "Success"},
            {"data", {
                {"name", "张三"},
                {"age", 30},
                {"email", "zhangsan@example.com"}
            }}
        };
    }

    json handleGetTime() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        char time_buf[64];
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now_time));

        return {
            {"code", 200},
            {"message", "Success"},
            {"data", {
                {"timestamp", static_cast<long>(now_time)},
                {"iso_time", time_buf}
            }}
        };
    }

    json handleCalculate(const json& data) {
        int a = data.at("a").get<int>();
        int b = data.at("b").get<int>();

        return {
            {"code", 200},
            {"message", "Success"},
            {"data", {
                {"operation", "addition"},
                {"result", a + b}
            }}
        };
    }

    std::unordered_map<std::string, ApiFunction> apiMap;
};

class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "wxWebView + Resource Protocol") {
        // 初始化WebView
        InitWebView();
    }

    void InitWebView() {
        wxString backend = wxWebViewBackendDefault;
        if (wxWebView::IsBackendAvailable(wxWebViewBackendEdge)) {
            backend = wxWebViewBackendEdge;
        } else if (wxWebView::IsBackendAvailable(wxWebViewBackendWebKit)) {
            backend = wxWebViewBackendWebKit;
        }

        webview = wxWebView::New(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                wxSize(800, 600), backend, wxBORDER_NONE);

        if (!webview) {
            spdlog::error("无法创建WebView组件，请检查配置");
            Close(true);
            return;
        }

        // 注册资源处理器
        webview->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new AppSchemeHandler()));

        webview->SetUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");

        webview->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &MyFrame::OnScriptMessage, this, webview->GetId());
        webview->AddScriptMessageHandler("wx");
        webview->Bind(wxEVT_WEBVIEW_ERROR, &MyFrame::OnWebViewError, this);
        webview->Bind(wxEVT_WEBVIEW_LOADED, &MyFrame::OnWebViewLoaded, this);

        // 直接加载嵌入式资源
        webview->LoadURL("app://index.html");

        webview->EnableContextMenu(true);
        webview->EnableAccessToDevTools(true);

        SetSize(800, 600);
        Center();
        Show();
    }

    void OnScriptMessage(wxWebViewEvent& evt) {
        wxString handler = evt.GetMessageHandler();
        if (handler == "wx") {
            try {
                std::string requestJson = evt.GetString().ToUTF8().data();
                json request = json::parse(requestJson);

                // 提取请求ID
                int requestId = request.value("__id", -1);

                std::string response = apiHandler.handleRequest(request);

                // 在响应中包含请求ID
                wxString script = wxString::Format(
                    "window.wxApi.resolveInvoke('%s', %d);",
                    EscapeJavascriptString(response),
                    requestId
                );
                webview->RunScript(script);
            }
            catch (const std::exception& e) {
                json errorResponse = {
                    {"code", 500},
                    {"message", e.what()},
                    {"data", json::object()}
                };

                // 尝试提取请求ID
                int requestId = -1;
                try {
                    json request = json::parse(evt.GetString().ToUTF8().data());
                    requestId = request.value("__id", -1);
                } catch (...) {}

                wxString errorScript = wxString::Format(
                    "window.wxApi.rejectInvoke('%s', %d);",
                    EscapeJavascriptString(errorResponse.dump()),
                    requestId
                );
                webview->RunScript(errorScript);
            }
        }
    }

    void OnWebViewError(wxWebViewEvent& evt) {
        spdlog::error("WebView错误: {}", evt.GetString().ToStdString());
    }

    void OnWebViewLoaded(wxWebViewEvent& evt) {
        spdlog::info("资源加载成功: {}", evt.GetURL().ToStdString());
    }

    wxString EscapeJavascriptString(const std::string& str) {
        wxString escaped = wxString::FromUTF8(str);
        escaped.Replace("\\", "\\\\");
        escaped.Replace("\"", "\\\"");
        escaped.Replace("\'", "\\\'");
        escaped.Replace("\n", "\\n");
        escaped.Replace("\r", "\\r");
        escaped.Replace("\t", "\\t");
        return escaped;
    }

private:
    wxWebView* webview;
    ApiHandler apiHandler;
};

class MyApp : public wxApp {
public:
    bool OnInit() override {
        // 创建日志目录
        wxString logDir = wxGetCwd() + wxFileName::GetPathSeparator() + "logs";
        if (!wxDirExists(logDir)) {
            wxMkdir(logDir);
        }

        // 设置日志文件路径
        wxString logPath = logDir + wxFileName::GetPathSeparator() + "app.log";

        try {
            // 创建旋转文件日志记录器（最大5MB，保留3个备份）
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logPath.ToStdString(), 1024 * 1024 * 5, 3);

            // 创建控制台日志记录器
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

            // 创建组合日志记录器
            std::vector<spdlog::sink_ptr> sinks{file_sink, console_sink};
            auto logger = std::make_shared<spdlog::logger>("main_logger", sinks.begin(), sinks.end());

            // 设置日志格式（精确到毫秒）
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

            // 设置日志级别
            logger->set_level(spdlog::level::trace);

            // 注册为默认日志记录器
            spdlog::set_default_logger(logger);

            spdlog::info("日志系统初始化成功，日志文件: {}", logPath.ToStdString());
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "日志初始化失败: " << ex.what() << std::endl;
            return false;
        }

        #ifdef __WXOSX__
            wxWebView::SetBackend(wxWebViewBackendWebKit);
        #endif

        MyFrame* frame = new MyFrame();
        return frame != nullptr;
    }

    int OnExit() override {
        // 程序退出时刷新并关闭日志记录器
        spdlog::shutdown();
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(MyApp);