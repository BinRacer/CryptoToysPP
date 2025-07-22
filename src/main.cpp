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
#include <mutex>
#include <queue>
#include <algorithm>
#include "resources.h"

using json = nlohmann::json;

// 增强的安全协议处理器
class AppSchemeHandler : public wxWebViewHandler {
public:
    AppSchemeHandler() : wxWebViewHandler("app") {}

    virtual wxFSFile* GetFile(const wxString& uri) {
        try {
            // 解析请求路径并执行安全验证
            ValidationResult pathResult = ValidateResourcePath(uri);
            if (!pathResult.valid) {
                spdlog::warn("路径验证失败: {} - {}", uri.ToStdString(), pathResult.message);
                return nullptr;
            }

            const std::string key = pathResult.safePath;
            spdlog::info("资源请求: {}", key);

            // 安全获取资源描述符
            ResourceDescriptor descriptor = GetResourceDescriptor(key);
            if (!descriptor.valid) {
                spdlog::warn(descriptor.message);
                return nullptr;
            }

            // 边界检查和零拷贝访问
            if (!CheckResourceBounds(descriptor)) {
                spdlog::error("资源边界检查失败: {} [偏移={}, 长度={}, 总大小={}]",
                              key, descriptor.offset, descriptor.length, resources::RESOURCE_DATA.size);
                return nullptr;
            }

            // 安全创建内存流
            return CreateSecureMemoryStream(key, descriptor);
        }
        catch (const std::exception& e) {
            spdlog::error("资源处理异常: {} - {}", uri.ToStdString(), e.what());
            return nullptr;
        }
    }

private:
    struct ValidationResult {
        bool valid = false;
        std::string safePath;
        std::string message;
    };

    struct ResourceDescriptor {
        bool valid = false;
        size_t offset;
        size_t length;
        std::string message;
    };

    // 路径安全验证函数
    ValidationResult ValidateResourcePath(const wxString& uri) {
        ValidationResult result;
        wxString path = uri.AfterFirst(':');

        // 移除多余斜杠
        path = path.Trim(false).Trim(true);
        while (path.StartsWith("/")) path = path.Mid(1);
        if (path.IsEmpty()) {
            result.message = "请求路径为空";
            return result;
        }

        // 阻止目录遍历攻击
        if (path.Contains("..") || path.Contains("//")) {
            result.message = "路径包含非法序列";
            return result;
        }

        // 检查扩展名白名单
        wxString ext = wxFileName(path).GetExt().Lower();
        static const std::vector<wxString> allowedExts = {
            "html", "js", "css", "png", "jpg", "jpeg", "gif", "json", "ico"
        };
        if (std::find(allowedExts.begin(), allowedExts.end(), ext) == allowedExts.end()) {
            result.message = "不支持的资源类型: " + ext.ToStdString();
            return result;
        }

        // 修正路径处理: 分开替换操作和字符串构建
        path.Replace("\\", "/", true);  // 先执行替换操作
        result.safePath = "/" + path.ToStdString();  // 然后转换为字符串
        result.valid = true;
        return result;
    }

    // 安全获取资源描述符
    ResourceDescriptor GetResourceDescriptor(const std::string& key) {
        ResourceDescriptor descriptor;
        auto lock = std::lock_guard<std::mutex>(resource_map_mutex);

        auto it = resources::RESOURCE_MAP.find(key);
        if (it == resources::RESOURCE_MAP.end()) {
            descriptor.message = "资源未找到: " + key;
            return descriptor;
        }

        descriptor.offset = it->second.first;
        descriptor.length = it->second.second;
        descriptor.valid = true;
        return descriptor;
    }

    // 资源边界安全检查
    bool CheckResourceBounds(const ResourceDescriptor& descriptor) {
        if (descriptor.offset >= resources::RESOURCE_DATA.size) {
            return false;
        }
        if (descriptor.length == 0) {
            return false;
        }
        if (descriptor.offset + descriptor.length > resources::RESOURCE_DATA.size) {
            return false;
        }
        return true;
    }

    // 安全创建内存流
    wxFSFile* CreateSecureMemoryStream(const std::string& key,
                                      const ResourceDescriptor& descriptor) {
        // 安全获取数据指针
        const uint8_t* data_ptr = resources::RESOURCE_DATA.data + descriptor.offset;

        // 创建具有边界保护的输入流
        auto stream = std::make_unique<wxMemoryInputStream>(
            reinterpret_cast<const char*>(data_ptr),
            static_cast<size_t>(descriptor.length)
        );

        // 验证流完整性
        if (!stream->IsOk() || stream->GetSize() != descriptor.length) {
            throw std::runtime_error("资源流创建失败");
        }

        return new wxFSFile(
            stream.release(),
            "app:/" + key,
            GetMimeType(key),
            wxEmptyString,
            wxDateTime::Now()
        );
    }

    wxString GetMimeType(const std::string& path) {
        if (path.find(".html") != std::string::npos) return "text/html";
        if (path.find(".js") != std::string::npos) return "application/javascript";
        if (path.find(".css") != std::string::npos) return "text/css";
        if (path.find(".png") != std::string::npos) return "image/png";
        if (path.find(".jpg") != std::string::npos) return "image/jpeg";
        if (path.find(".json") != std::string::npos) return "application/json";
        if (path.find(".ico") != std::string::npos) return "image/x-icon";
        return "application/octet-stream";
    }

    std::mutex resource_map_mutex;
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
            if (!request.contains("path")) {
                return CreateErrorResponse(400, "缺少必要参数: path");
            }

            std::string path = request["path"].get<std::string>();
            json data = request.value("data", json::object());

            // API访问频率限制（基本实现）
            if (!CheckRateLimit(path)) {
                return CreateErrorResponse(429, "请求过于频繁");
            }

            auto it = apiMap.find(path);
            if (it != apiMap.end()) {
                response = it->second(data);
            } else {
                response = CreateErrorResponse(404, "未找到API: " + path);
            }
        }
        catch (const std::exception& e) {
            response = CreateErrorResponse(500, "内部错误: " + std::string(e.what()));
        }
        catch (...) {
            response = CreateErrorResponse(500, "未知内部错误");
        }

        return response.dump();
    }

private:
    json CreateErrorResponse(int code, const std::string& message) {
        return {
            {"code", code},
            {"message", message},
            {"data", json::object()}
        };
    }

    bool CheckRateLimit(const std::string& path) {
        auto now = std::chrono::steady_clock::now();
        auto& record = rateLimits[path];

        // 清除过期记录
        while (!record.empty() &&
               std::chrono::duration_cast<std::chrono::seconds>(now - record.front()).count() > TIME_WINDOW) {
            record.pop();
        }

        // 检查次数限制
        if (record.size() >= MAX_REQUESTS) {
            return false;
        }

        record.push(now);
        return true;
    }

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

    // 限流设置
    static constexpr int MAX_REQUESTS = 100;  // 每API最大请求数
    static constexpr int TIME_WINDOW = 60;    // 时间窗口(秒)
    std::unordered_map<std::string, std::queue<std::chrono::steady_clock::time_point>> rateLimits;
};

class MyFrame : public wxFrame {
public:
    MyFrame() : wxFrame(nullptr, wxID_ANY, "wxWebView + Optimized Resource Protocol") {
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

        webview->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new AppSchemeHandler()));
        webview->SetUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");
        webview->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &MyFrame::OnScriptMessage, this, webview->GetId());
        webview->AddScriptMessageHandler("wx");
        webview->Bind(wxEVT_WEBVIEW_ERROR, &MyFrame::OnWebViewError, this);
        webview->Bind(wxEVT_WEBVIEW_LOADED, &MyFrame::OnWebViewLoaded, this);
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
                int requestId = request.value("__id", -1);
                std::string response = apiHandler.handleRequest(request);

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
        wxString logDir = wxGetCwd() + wxFileName::GetPathSeparator() + "logs";
        if (!wxDirExists(logDir)) wxMkdir(logDir);

        wxString logPath = logDir + wxFileName::GetPathSeparator() + "app.log";
        try {
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logPath.ToStdString(), 1024 * 1024 * 5, 3);
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            std::vector<spdlog::sink_ptr> sinks{file_sink, console_sink};
            auto logger = std::make_shared<spdlog::logger>("main_logger", sinks.begin(), sinks.end());
            logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
            logger->set_level(spdlog::level::trace);
            spdlog::set_default_logger(logger);
            spdlog::info("日志系统初始化成功");
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
        spdlog::shutdown();
        return wxApp::OnExit();
    }
};

wxIMPLEMENT_APP(MyApp);
