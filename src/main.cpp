#include <wx/wx.h>
#include <wx/webview.h>
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

using json = nlohmann::json;

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
            std::string method = request.at("method").get<std::string>();
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
    MyFrame() : wxFrame(nullptr, wxID_ANY, "wxWebView + Vue3") {
        wxString backend = wxWebViewBackendDefault;
        if (wxWebView::IsBackendAvailable(wxWebViewBackendEdge)) {
            backend = wxWebViewBackendEdge;
        } else if (wxWebView::IsBackendAvailable(wxWebViewBackendWebKit)) {
            backend = wxWebViewBackendWebKit;
        }

        webview = wxWebView::New(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                wxSize(800, 600), backend, wxBORDER_NONE);

        if (!webview) {
            wxMessageBox("无法创建WebView组件，请检查配置", "初始化错误", wxOK | wxICON_ERROR);
            Close(true);
            return;
        }

        webview->EnableContextMenu(false);
        webview->SetUserAgent("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36");

        webview->Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &MyFrame::OnScriptMessage, this, webview->GetId());
        webview->AddScriptMessageHandler("wx");
        webview->Bind(wxEVT_WEBVIEW_ERROR, &MyFrame::OnWebViewError, this);

        CreateHtmlFile();
        webview->LoadURL(wxFileName(tempHtmlPath).GetFullPath());

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

                // 关键修复：在响应中包含请求ID
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
        wxLogError("WebView错误: %s", evt.GetString());
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

    ~MyFrame() {
        if (wxFile::Exists(tempHtmlPath)) {
            wxRemoveFile(tempHtmlPath);
        }
    }

private:
    wxWebView* webview;
    ApiHandler apiHandler;
    wxString tempHtmlPath;

    void CreateHtmlFile() {
        wxString tempDir = wxStandardPaths::Get().GetTempDir();
        tempHtmlPath = wxFileName(tempDir, "wxwebview_vue_app.html").GetFullPath();

        wxFile htmlFile;
        if (htmlFile.Create(tempHtmlPath, true)) {
            wxString content = GenerateHtmlContent();
            wxScopedCharBuffer utf8Buffer = content.utf8_str();
            htmlFile.Write(utf8Buffer.data(), utf8Buffer.length());
            htmlFile.Close();
        }
    }

    wxString GenerateHtmlContent() {
        return wxString::FromUTF8(R""""(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>wxWebView + Vue3 JSON通信</title>
    <script src="https://unpkg.com/vue@3/dist/vue.global.js"></script>
    <style>
        body { font-family: Arial, sans-serif; padding: 20px; background-color: #f5f5f5; }
        button { margin: 5px; padding: 8px 12px; background: #4a6fa5;
                 color: white; border: none; border-radius: 4px; cursor: pointer; }
        button:hover { background: #3a5a80; }
        pre { background: #f0f0f0; padding: 10px; border-radius: 4px;
              white-space: pre-wrap; word-wrap: break-word; }
        .error { color: #d32f2f; }
        .success { color: #388e3c; }
        .request-box, .response-box { margin: 20px 0; padding: 15px;
                                     border: 1px solid #ddd; border-radius: 5px; background: white; }
        .request-box h3, .response-box h3 { margin-top: 0; }
        .container { max-width: 800px; margin: 0 auto; }
        #app { padding: 20px; }
        h1 { color: #2c3e50; text-align: center; }
    </style>
</head>
<body>
    <div class="container">
        <div id="app">
            <h1>C++与Vue3 JSON通信</h1>

            <div class="request-box">
                <button @click="callApi('getUser')">获取用户信息</button>
                <button @click="callApi('getTime')">获取服务器时间</button>
                <button @click="callApi('calculate', { a: 5, b: 3 })">计算(5+3)</button>
                <button @click="callApi('calculate', { a: 10, b: 7 })">计算(10+7)</button>

                <div style="margin-top: 15px;">
                    <h3>请求体:</h3>
                    <pre>{{ requestJson }}</pre>
                </div>
            </div>

            <div class="response-box" v-if="loading">
                <h3>状态: <span class="success">加载中...</span></h3>
            </div>

            <div class="response-box" v-if="response">
                <h3>响应:
                    <span :class="response.code >= 400 ? 'error' : 'success'">
                        {{ response.code }} {{ response.message }}
                    </span>
                </h3>
                <pre>{{ prettyJson(response.data) }}</pre>
            </div>

            <div class="response-box" v-if="error">
                <h3 class="error">错误</h3>
                <pre>{{ error }}</pre>
            </div>
        </div>
    </div>

    <script>
        console.log("页面已加载");
        console.log("wx对象:", typeof wx !== 'undefined' ? wx : '未定义');

        // 创建全局API对象
        window.wxApi = (function() {
            const pendingPromises = {};
            let nextId = 0;

            function invoke(requestObj) {
                const id = nextId++;

                return new Promise((resolve, reject) => {
                    pendingPromises[id] = { resolve, reject };

                    const requestWithId = {
                        ...requestObj,
                        __id: id
                    };

                    if (typeof wx === 'undefined') {
                        console.error("wx对象未定义，无法发送消息");
                        reject({ message: "wx对象未定义" });
                        return;
                    }

                    // 发送消息
                    wx.postMessage(JSON.stringify(requestWithId));
                });
            }

            // 关键修复：添加ID参数
            function resolveInvoke(responseJson, id) {
                try {
                    const response = JSON.parse(responseJson);

                    if (pendingPromises[id]) {
                        pendingPromises[id].resolve(response);
                        delete pendingPromises[id];
                    }
                } catch (e) {
                    console.error("解析响应错误:", e);
                }
            }

            // 关键修复：添加ID参数
            function rejectInvoke(errorJson, id) {
                try {
                    const error = JSON.parse(errorJson);

                    if (pendingPromises[id]) {
                        pendingPromises[id].reject(error);
                        delete pendingPromises[id];
                    }
                } catch (e) {
                    console.error("解析错误消息失败:", e);
                }
            }

            return {
                invoke,
                resolveInvoke,
                rejectInvoke
            };
        })();

        // 创建Vue应用
        const { createApp, ref, computed, onMounted } = Vue;

        createApp({
            setup() {
                const response = ref(null);
                const error = ref(null);
                const loading = ref(false);
                const currentRequest = ref({
                    path: "",
                    method: "",
                    data: {}
                });

                onMounted(() => {
                    console.log("Vue应用已挂载");
                });

                const requestJson = computed(() => {
                    return JSON.stringify(currentRequest.value, null, 2);
                });

                function callApi(endpoint, data = null) {
                    currentRequest.value = {
                        path: `/api/${endpoint}`,
                        method: data ? 'POST' : 'GET',
                        data: data || {}
                    };

                    loading.value = true;
                    error.value = null;
                    response.value = null;

                    window.wxApi.invoke(currentRequest.value)
                        .then(result => {
                            response.value = result;
                            if (result.code >= 400) {
                                error.value = `错误 ${result.code}: ${result.message}`;
                            }
                        })
                        .catch(err => {
                            error.value = err.message || "未知通信错误";
                            if (err.data) {
                                error.value += `\n${JSON.stringify(err.data, null, 2)}`;
                            }
                        })
                        .finally(() => {
                            loading.value = false;
                        });
                }

                function prettyJson(obj) {
                    return JSON.stringify(obj, null, 2);
                }

                return {
                    response,
                    error,
                    loading,
                    currentRequest,
                    requestJson,
                    callApi,
                    prettyJson
                };
            }
        }).mount('#app');
    </script>
</body>
</html>
)"""");
    }
};

class MyApp : public wxApp {
public:
    bool OnInit() override {
        wxLog::SetLogLevel(wxLOG_Max);
        wxLog::EnableLogging();

        #ifdef __WXOSX__
            wxWebView::SetBackend(wxWebViewBackendWebKit);
        #endif

        MyFrame* frame = new MyFrame();
        return frame != nullptr;
    }
};

wxIMPLEMENT_APP(MyApp);