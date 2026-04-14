# 酒店房间预定管理系统（链表版）

这是一个使用 **C 语言 + 单链表** 实现的命令行酒店系统，支持角色权限、分段预约、付款锁定和本地持久化。

## 当前功能

- 角色权限分离：`管理员` 与 `顾客`
- 房间管理：查看、添加、删除
- 预约管理：按时间段（年月日）预约、取消、查询
- 付款流程：预约后先锁单，支持单独付款
- 5 分钟锁单过期：未付款预约自动失效
- 入住/退房：入住前校验付款，入住时要求填写入住人信息
- 统计信息：房间数、预约数、入住数、已付款数、锁定数、预估收入

## 预约时间规则

- 预约时间使用 **年/月/日** 输入
- 采用区间语义：`[开始日期, 结束日期)`，结束日期为退房日
- 同一房间不允许时间段重叠

## 角色权限

- 管理员：查看房间、查空房、预约、付款、取消预约、入住、退房、按姓名查询、统计、添加房间、删除房间
- 顾客：查看房间、查空房、预约、付款、入住、退房、按姓名查询

## 持久化数据（data_house）

- 启动时优先从 `data_house` 读取历史数据
- 若读取失败，则使用默认初始化房间
- 退出时将当前内存数据保存到 `data_house`
- 当前文件：
  - `data_house/rooms.db`
  - `data_house/reservations.db`

## 目录结构（核心）

- `main.c`：程序入口、角色会话、启动加载与退出保存
- `information/src/hotel.c` + `information/inc/hotel.h`：链表业务逻辑（预约/付款/入住/统计）
- `information/src/hotel_storage.c` + `information/inc/hotel_storage.h`：数据读写
- `information/src/role.c` + `information/inc/role.h`：角色权限分发
- `information/src/menu.c` + `information/inc/menu.h`：菜单显示与输入

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

如已存在可执行文件，可直接运行：

```powershell
.\run-utf8.ps1 -SkipBuild
```

> 菜单输入 `0` 可退出，退出时会自动保存数据。
