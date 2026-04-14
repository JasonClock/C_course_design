# 酒店房间预定管理系统（链表版）

这是一个使用 **C 语言 + 单链表** 实现的命令行酒店房间预定管理系统。

## 功能

- 查看全部房间、查看空闲房间
- 预定房间、取消预定
- 入住办理、退房结算
- 按客人姓名关键字查询
- 统计空闲/预定/入住数量与预估营业额
- 动态添加房间、删除房间（链表插入/删除）

## 数据结构说明

- 使用 `Room` 结构体表示一个房间节点
- 节点之间通过 `next` 指针组成单链表
- 链表维护房间状态：空闲、已预定、已入住

## 分层结构

- `main.c`：程序入口与流程分发
- `information/src/menu.c` + `information/inc/menu.h`：菜单展示与菜单选择读取
- `information/src/hotel.c` + `information/inc/hotel.h`：房间链表与业务逻辑
- `information/src/input.c` + `information/inc/input.h`：通用输入处理（int/double/string）

## 构建与运行

```powershell
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug
.\cmake-build-debug\C.exe
```

## UTF-8 运行（推荐）

```powershell
.\run-utf8.ps1
```

如果你当前环境暂时无法构建，但已有 `cmake-build-debug\C.exe`，可以跳过构建直接运行：

```powershell
.\run-utf8.ps1 -SkipBuild
```

> 可在菜单中输入 `0` 退出系统。

## 说明

- 默认初始化了 8 个房间
- 所有数据保存在内存中，程序退出后不会持久化
