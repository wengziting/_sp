# 🌊 FluxScript

![Version](https://img.shields.io/badge/version-0.1.0--alpha-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Python](https://img.shields.io/badge/python-3.8%2B-blue)

**FluxScript** 是一種極簡、意圖導向（Intent-oriented）的微型程式語言。它設計的初衷是為了讓「數據流處理」變得像流水一樣直觀，將複雜的邏輯簡化為一系列的管道操作。

## ✨ 特性
- **極簡語法**：沒有贅餘的變數宣告，只有數據的流動。
- **管道運算**：使用 `|>` 運算子，模擬數據經過過濾與轉換的過程。
- **清晰意圖**：使用 `->` 符號定義數據的最終歸宿（Sink）。
- **輕量級**：核心解譯器僅使用 Python 標準庫實作，無須額外依賴。

## 🛠 語法導覽

在 FluxScript 中，一段典型的程式碼如下：
```flux
[1, 2, 3, 4, 5] |> sum -> output
