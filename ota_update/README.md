# GD32F407 OTA / IAP 自动升级示例

基于 `GD32F407VET6`（512KB Flash，8MHz HXTAL → 168MHz）的 **A/B 双区 + 回滚** 引导加载器（Bootloader）与演示应用（App），OTA 下载走 **USART + YMODEM(1K, CRC16)**。

工程配置完全仿照 `LiftV1.6/MCUTemplate`（器件 `GD32F407VE`、DFP `GD32F4xx_DFP 3.0.3`、预定义宏 `GD32F40X`、ARMCC AC5、`__SYSTEM_CLOCK_168M_PLL_8M_HXTAL`）。

---

## 一、目录结构

```
ota_update/
├── common/                 # 共享代码（分区常量 + IAP 核心），两个工程都引用
│   ├── ota_config.h        #   唯一的分区/地址定义来源
│   ├── ota_crc.h/.c        #   CRC16(XMODEM) + CRC32(镜像校验)
│   ├── ota_flash.h/.c      #   FMC 扇区擦除/按字写入/校验
│   ├── bootmgr.h/.c        #   启动记录读写 + 跳转
│   ├── ymodem.h/.c         #   YMODEM 接收器 (跑在下载口 USART1)
│   ├── ota_log.h/.c        #   日志口 USART0 (PA9/PA10)
│   └── ota_dl.h/.c         #   下载口 USART1 (PA2/PA3)
├── Bootloader/             # IAP 引导器工程 (链接 0x08000000)
├── App/                    # 演示 App 工程 (两个 Target: App_A / App_B)
└── tools/ymodem_send.py    # 上位机下发工具 (hex->bin + YMODEM)
```

---

## 二、Flash 分区（512KB, 0x08000000 ~ 0x0807FFFF）

| 区域 | 扇区 | 大小 | 地址范围 |
|---|---|---|---|
| **Bootloader** | 0~2 | 48KB | `0x08000000` ~ `0x0800BFFF` |
| **BootMgr** | 3 | 16KB | `0x0800C000` ~ `0x0800FFFF`（存放启动记录） |
| **Bank A** | 4~5 | 192KB | `0x08010000` ~ `0x0803FFFF` |
| **Bank B** | 6~7 | 256KB | `0x08040000` ~ `0x0807FFFF`（示例只用前 192KB） |

- `APP_MAX_SIZE = 192KB`（取 Bank A/B 较小者）
- 分区常量只定义在 [`common/ota_config.h`](common/ota_config.h) 一处，Bootloader 与 App **必须一致**。
- App 镜像 A/B 两端分别链接到 `0x08010000` / `0x08040000`，靠 `SCB->VTOR = APP_ADDR` 重定位向量表。

---

## 三、工作流程

### 启动记录（BootMgr 扇区）
```c
typedef struct {
  uint32_t magic;         // "OTAB"
  uint32_t active_slot;   // SLOT_A / SLOT_B
  uint32_t state[2];      // ST_INVALID / ST_CONFIRMED / ST_TRYING
  uint32_t try_count;
  uint32_t request;       // REQ_NONE / REQ_UPDATE
  uint32_t app_size[2];
  uint32_t app_crc[2];    // 整镜像 CRC32
} boot_record_t;
```

### Bootloader 启动流程
1. 读启动记录；无有效记录则初始化（A/B 均 `INVALID`）
2. 若 `request == REQ_UPDATE` → 进入 OTA
3. 活动槽为 `CONFIRMED` → 校验 CRC32 → 跳转；CRC 不符则进 OTA
4. 活动槽为 `TRYING` → CRC 通过则 `try_count++` 并跳转；累计 `TRY_LIMIT(5)` 次仍未被 App 确认 → **回滚**到另一槽
5. 无有效 App → 进入 OTA

### OTA 下载（总烧到**非活动槽**，保留可回滚的好版本）
1. 擦除非活动槽（`ota_flash_erase`, FMC 按扇区）
2. `ymodem_receive()` 逐包写入 + CRC 校验
3. 整镜像 CRC32 → 写入启动记录（目标槽 `ST_TRYING`, `active_slot` 切换）→ 复位

### App 侧
- `main()` 首行 `SCB->VTOR = APP_ADDR`（A/B 任意槽皆可运行）
- 正常启动后调用 `bootmgr_confirm()` 标记本槽 `CONFIRMED`
- 串口收到字符 `'O'` → `bootmgr_request_update()` + 复位 → 回到 Bootloader 进入 OTA
- 设 `APP_SIMULATE_BAD=1` 可模拟"坏固件"（启动后不确认），观察回滚

---

## 四、编译

1. Keil MDK 安装 `GigaDevice.GD32F4xx_DFP 3.0.3`（与模板一致）
2. 打开 `Bootloader/USER/Bootloader.uvprojx` → Build → 产出 `Bootloader\OBJ\Bootloader.hex`
3. 打开 `App/USER/App.uvprojx` → 依次构建两个 Target：
   - `App_A`（链接 `0x08010000`）→ `App\OBJ\App_A.hex`
   - `App_B`（链接 `0x08040000`）→ `App\OBJ\App_B.hex`

> 两个 Target 仅链接地址与 `-DAPP_ADDR` 不同，源码共用。

---

## 五、烧录与测试

### 首次烧录
1. 用烧录器烧录 `Bootloader.hex` 到 `0x08000000`
2. 用 `App_A.hex` 做首次 OTA（此时 A/B 都无效，Bootloader 上电即等待 YMODEM）

### OTA 下发
```bash
pip install pyserial
python tools/ymodem_send.py --port COM3 --baud 9600 --file App_A.hex
```

### 完整演示流程
1. 上电 → Bootloader 发现无有效 App → 串口打印 `Waiting YMODEM ...`
2. 运行 `ymodem_send.py` 发送 `App_A.hex`
3. 收到后复位，跳转 Bank A，App 打印 `Boot confirmed.` 并闪灯
4. 在串口输入 `O` → App 请求升级并复位 → Bootloader 进入 OTA
5. 发送 `App_B.hex` → 新固件写入 **Bank B**，跳转 Bank B 运行
6. 在 App 中把 `APP_SIMULATE_BAD` 置 1 重新烧录 → 连续 5 次启动未确认 → 自动回滚到上一好版本

---

## 六、注意事项

1. **时钟必须匹配**：本示例假设板载 8MHz HXTAL（与 LiftV1.6 一致）。若实体板晶振不同，需同步改 `App/USER/gd32f4xx.h` 的 `HXTAL_VALUE` 与 `system_gd32f4xx.c` 的 PLL 档位。
2. **日志口 USART0 (PA9/PA10)**：打印调试信息、接收 'O' 升级命令；**下载口 USART1 (PA2/PA3)**：YMODEM 收 hex。两路串口独立，本工程不含 Modbus，无冲突。
3. **FMC 擦除粒度**：GD32F407 **只有扇区擦除**（`fmc_page_erase` 仅 425/427/470 可用），所以分区按扇区边界对齐，最小 16KB。
4. **A/B 需两个链接地址**：位置相关的 ARM 代码无法在同一份二进制里跑两个地址，故 App 提供 `App_A`/`App_B` 两个 Target。升级时**发给哪个槽就用对应地址的 hex**。
5. **与 LiftV1.6 既有代码的一个隐患**：`bll_main.c` 里 `app_addr = 0x080e0000`（≈896KB）在 512KB 的 407VET6 上是**越界**的，跳转会进 HardFault —— 本示例使用正确的 `0x08010000/0x08040000`，但建议你回头修一下那个值。
6. `tools/ymodem_send.py` 会把 `.hex` 转成纯二进制（间隙填 0xFF）再发送；Bootloader 记录**整镜像 CRC32**，每次启动都会校验，能拦住半截/损坏的固件。