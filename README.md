# armv7m_DWT

armv7m_DWT: 基于 ARMV7M 架构设计的数据地址监控组件。

利用 CoreSight 的调试功能，实时监控每一次内存读写访问，必要时你可以在 `DebugMon_Handler` 调试中断里将访问内存时的现场打印出来，抓住**踩踏内存**的元凶！

如果觉得好用请点小星星，小星星就是对我最大的支持！（^_^）

## 1. 目录结构

| 目录 | 说明                                              |
| ---- | ------------------------------------------------- |
| docs | 包含移植，原理等相关说明。                        |
| port | 移植文件（接管 debug 中断，输出打印，示例代码等） |
| src  | 核心源码，移植过程当中不需要修改。                |

## 2. API 说明

#### 2.1 初始化 dwt

此函数将会使能 DWT 和 ITM 单元，并开启 DebugMonitor 的中断。

```c
int dwt_init(void);
```
#### 2.2 反初始化 dwt

此函数将会失能 DWT 和 ITM 单元，关闭 DebugMonitor 中断，并将配置的参数清零。

```c
int dwt_deinit(void)
```

#### 2.3 启动数据地址访问监控

此函数将会使用一个通道，启动对 `addr` 指向的内存地址的监控。一旦有线程或函数访问了此地址，就会产生 `DebugMon_Handler` 中断。

```c
int data_address_watch_start(int ch, void *addr, uint32_t addr_mask, enum DWT_MODE mode);
```

| 参数      | 描述                                                         |
| --------- | ------------------------------------------------------------ |
| ch        | 选择要使用的监控通道。                                       |
| addr      | 要监控的内存地址。                                           |
| addr_mask | 内存地址掩码。配合掩码可对一片内存地址进行监控。(注意：需小于等于 **0x0000001F**) |
| mode      | 监控模式，包含三种模式：1.读内存监控；2.写内存监控；3.读写内存监控； |

#### 2.4 停止数据地址访问监控

此函数将会停止一个通道的监控功能，与 `data_address_watch_start` 成对使用。

```c
int data_address_watch_stop(int ch)
```

| 参数 | 描述                   |
| ---- | ---------------------- |
| ch   | 选择要关闭的监控通道。 |

#### 2.5 启动数据内容匹配监控

此函数将会使用多个通道，启动对最多两个内存地址上的内容的监控。一旦监控地址上的值与 `compare_value` 相等，就会产生一次 `DebugMon_Handler` 中断。

```c
int data_value_watch_start(int ch, void *addr1, void *addr2, void *compare_value, enum DWT_DATA_SIZE data_size)
```

| 参数          | 描述                                         |
| ------------- | -------------------------------------------- |
| ch            | 选择要使用的监控通道。                       |
| addr1         | 监控的内存地址1。（可填入 NULL ）            |
| addr2         | 监控的内存地址2。（可填入 NULL ）            |
| compare_value | 比较值。                                     |
| data_size     | 数据类型。（1. byte；2. half word；3. word） |

#### 2.6 停止数据内容匹配监控

此函数将会停止一个通道的监控功能，与 `data_value_watch_start` 成对使用。

```c
int data_value_watch_stop(int ch)
```

| 参数 | 描述                   |
| ---- | ---------------------- |
| ch   | 选择要关闭的监控通道。 |

## 3. 使用流程

#### 3.1 内存地址监控

```c
int dwt_address_test(int argc, char **argv)
{
    int ch;
    int ret;
    volatile rt_uint8_t temp;
    temp = 0x55;
    databuf[1] = 0x00 ;

    /* 1.第一步 */
    dwt_init();
    ch = 1;
    /* 2.第二步 */
    // ret = data_address_watch_start(ch, (void*)&databuf[1], 0x00000000, READ_ONLY);
    ret = data_address_watch_start(ch, (void*)&databuf[1], 0x00000000, READ_WRITE);
    // ret = data_address_watch_start(ch, (void*)&databuf[1], 0x00000000, WRITE_ONLY);
    if(ret != 0)
    {
        return 0;
    }
    rt_kprintf("1. temp = databuf[1];\r\n");
    rt_thread_mdelay(500);
    temp = databuf[1];

    rt_kprintf("2. databuf[1] = 0x55 ;\r\n");
    rt_thread_mdelay(500);
    databuf[1] = 0x55 ;

    /* 3.第三步 */
    data_address_watch_stop(ch);
    /* 4.第四步 */
    dwt_deinit();

    return 0;
}
```

#### 3.1 内存数值监控

```c
int dwt_value_test(int argc, char **argv)
{
    int ch;
    int ret;
    volatile rt_uint8_t temp;
    temp = 0x55;
    databuf[1] = 0x00 ;
    databuf[2] = 0x00 ;

    /* 1.第一步 */
    dwt_init();
    ch = 1;
    /* 2.第二步 */
    ret = data_value_watch_start(ch, (void*)&databuf[1], (void*)&databuf[2], (void*)&temp, SIZE_BYTE);
    // ret = data_value_watch_start(ch, (void*)NULL, (void*)&databuf[2], (void*)&temp, SIZE_BYTE);
    if(ret != 0)
    {
        return 0;
    }

    rt_kprintf("1. temp = databuf[1];\r\n");
    rt_thread_mdelay(500);
    temp = databuf[1];
    rt_kprintf("2. temp = databuf[2];\r\n");
    rt_thread_mdelay(500);
    temp = databuf[2];

    rt_kprintf("3. databuf[1] = 0x55 ;\r\n");
    rt_thread_mdelay(500);
    databuf[1] = 0x55 ;
    rt_kprintf("4. databuf[2] = 0x55 ;\r\n");
    rt_thread_mdelay(500);
    databuf[2] = 0x55 ;

    rt_thread_mdelay(500);

    rt_kprintf("5. temp = databuf[1];\r\n");
    rt_thread_mdelay(500);
    temp = databuf[1];
    rt_kprintf("6. temp = databuf[2];\r\n");
    rt_thread_mdelay(500);
    temp = databuf[2];


    rt_kprintf("7. databuf[1] = 0x56 ;\r\n");
    rt_thread_mdelay(500);
    databuf[1] = 0x56 ;
    rt_kprintf("8. databuf[2] = 0x56 ;\r\n");
    rt_thread_mdelay(500);
    databuf[2] = 0x56 ;

    /* 3.第三步 */
    data_value_watch_stop(ch);
    /* 4.第四步 */
    dwt_deinit();

    return 0;
}
```
