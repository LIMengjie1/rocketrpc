# rocketrpc

日志模块：
```
1:日志级别
2：打印到文件，支持日期命令，日志桂东
3：C格式化输出
4：线程安全
```

logLevel
```
Debug
Info
Error
```

日志格式
[level][%y-%m-%d %H:%M:%S.%ms\t[pid:thread_id]]\t[file_name:line]

Logger 
1:提供打印日志的方法
2：设置输出的路径