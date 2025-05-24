
# 📊 StockAnalysisPro

**StockAnalysisPro** 是一套完整的股票技術指標資料前後端系統，結合 **C++ 多執行緒伺服器** 與 **Qt GUI 客戶端**，實現股票 JSON 資料的傳輸、解析與視覺化。支援多股票同時接收、展示、技術分析圖表呈現，適合作為金融資料處理與視覺化學習專案範例。

---

## 🧱 專案結構

```
StockAnalysisPro/
├── server/                    # 伺服器端模組
│   ├── FileReader.h
│   ├── JsonPacket.h
│   ├── NetworkServer.h
│   ├── PacketFactory.h
│   ├── PacketInterface.h
│   ├── task_pool.h
│   └── main.cpp
│
├── client/                    # 客戶端模組
│   ├── main.cpp
│   ├── mainwindow.h/.ui
│   ├── stockdatamanager.h
│   ├── stockdatareader.h
│   ├── stockdatasocketreceiver.h
│   ├── stockdetailwindow.h
│   └── qcustomplot.h
│
├── diagrams/                  # UML 與流程圖
│   ├── system_overview.png
│   ├── module_diagram.png
│   ├── data_flow.png
│   ├── packet_structure.png
│   └── ...
│
└── README.md
```

---

## 🌐 系統架構概覽

本系統分為兩大部分：

### ✅ 伺服器端
- 讀取多支股票 JSON 技術指標資料
- 封裝為統一封包格式 (`JsonPacket`)
- 使用 TCP Socket 傳送給多客戶端
- 支援多執行緒傳輸 (`TaskPool`)

### ✅ 客戶端
- 即時接收伺服器資料 (`StockDataSocketReceiver`)
- 管理多股票數據 (`StockDataManager`)
- 使用表格 (`QTableWidget`) 展示即時資料
- 使用圖表 (`QCustomPlot`) 呈現 K 線圖、RSI、MACD
- 支援滑鼠十字線追蹤、縮放、詳細視窗 (`StockDetailWindow`)

---

## 🔩 功能說明

| 模組名稱                      | 功能描述                                    |
|-----------------------------|---------------------------------------------|
| `NetworkServer`             | 伺服器端 Socket 封裝，支援多客戶端傳輸       |
| `TaskPool`                  | 多執行緒池實現                               |
| `PacketInterface` / `JsonPacket` | 封包格式設計與實現                        |
| `StockDataReader`           | JSON 檔案讀取工具                            |
| `StockDataSocketReceiver`   | 客戶端 TCP 接收器，解析 JSON 封包            |
| `StockDataManager`          | 多股票資料管理，支援查詢與存取              |
| `StockDetailWindow`         | 詳細股票視窗，支援多指標圖表繪製             |
| `QCustomPlot`               | 技術指標繪圖元件 (K 線、RSI、MACD)           |

---

## 📦 UML 與系統圖

請參考 `diagrams/` 資料夾中的設計圖：
- 系統架構總覽圖
- 模組關聯圖
- 封包結構設計圖
- 資料處理流程圖

例如：

![System Overview](diagrams/system_overview.png)

---

## 🚀 使用方式

### 伺服器端

1️⃣ 編譯：
```bash
g++ main.cpp -o JsonServer.exe -lws2_32
```
2️⃣ 執行：
```bash
./JsonServer.exe
```
伺服器啟動後將監聽 `8080` 埠口。

---

### 客戶端

1️⃣ 使用 Qt Creator 或 CMake 編譯專案  
2️⃣ 執行 `StockAnalysisPro`，自動連接伺服器並接收資料。

---

## 📝 封包格式範例

```json
{
  "DataType": "JSON",
  "Payload": [
    { "symbol": "AAPL", "data": { ... } },
    { "symbol": "GOOGL", "data": { ... } },
    ...
  ]
}
```

---

## 🔧 待辦與改進方向

- [ ] 增加重連與錯誤處理機制
- [ ] 增加歷史資料儲存與查詢功能
- [ ] 增加更多技術指標圖表
- [ ] 支援用戶自訂欄位與篩選

---

如有需要，我可以協助修改補充或進一步產出其他文件，例如技術白皮書、API 文件等。 🚀
