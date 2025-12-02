# QuickNTP-zh_CN

---

### 使用 NTP 服务器列表更新您的 Nintendo Switch 时钟

使用自定义 DNS（或受限连接）时，可能无法通过 Nintendo 服务器更新时钟，导致 Switch 容易出现时间不同步的情况。

有了这款插件，您现在可以使用自定义的 [NTP](https://en.wikipedia.org/wiki/Network_Time_Protocol) 服务器快速更新时钟！

---

![Preview](https://github.com/user-attachments/assets/0218a5e6-92be-44ed-9de1-af400927a493)

## 功能

- 通过从服务器列表中选择来更新时间（[可自定义！](#customize-servers)）

- 显示当前时间与所选服务器的偏移量

- 将内部网络时间设置为用户在系统设置中设置的时间（穿越时空，耶！）

### 自定义服务器

自 v1.5.0 版本起，您可以使用以下语法创建或修改提供的 `quickntp.ini` 文件：

```ini
[Servers]
My_Ntp_Server = ntp.example.com
```

用户界面中下划线将被空格替换！

自制程序将在以下位置查找该文件（*将使用第一个有效文件*）：

- /config/quickntp.ini

- /config/quickntp/config.ini

- /switch/.overlays/quickntp.ini

**注意**：如果文件缺失或无效，则只会显示“NTP Pool Main”服务器。

## 未来可能的功能

- 改进错误处理/消息

- 显示带秒数的时钟

- 将心形图标转换为时钟（需要检查字体）

- 在单独的线程中获取时间

- 根据用户所在地区选择最近的 NTP 服务器

- 以毫秒为单位更新时间（可能受系统限制）
## Contributors

- [@DarkMatterCore](https://github.com/DarkMatterCore) (library updates)
- [@DraconicNEO](https://github.com/DraconicNEO) (new NTP servers)
- [@ppkantorski](https://github.com/ppkantorski) (Ultrahand version and 4MB heap support)

## Credits

### Special thanks

- [The 4TU Team](https://github.com/fortheusers) for freely hosting this app on their [Homebrew App Store](https://hb-app.store/)

### Code and libraries

- [@3096](https://github.com/3096) for `SwitchTime`, who gave me the initial idea and some code examples
- [@thedax](https://github.com/thedax) for `NX-ntpc`, used by [@3096](https://github.com/3096)
- [@SanketDG](https://github.com/SanketDG) for the [NTP Client](https://github.com/SanketDG/c-projects/blob/master/ntp-client.c) using `getaddrinfo` instead of `gethostbyname`
- [@WerWolv](https://github.com/WerWolv) for `libtesla`
- [@ITotalJustice](https://github.com/ITotalJustice) for `minIni-nx` (forked from [@compuphase](https://github.com/compuphase))

### Suggested NTP servers

- [NTP Pool Project](https://www.ntppool.org)
- [Cloudflare Time Services](https://www.cloudflare.com/time/)
- [Google Public NTP](https://developers.google.com/time)
- [NIST Internet Time Servers](https://tf.nist.gov/tf-cgi/servers.cgi)

## 故障排除

- 系统设置中应启用“通过互联网同步时钟”选项（具体操作请参见此处：https://en-americas-support.nintendo.com/app/answers/detail/a_id/22557/p/989/c/188），因为此程序会更改“网络时钟”（该设置不可更改）。

- 如果仅显示“NTP Pool Main”服务器，则说明 SD 卡“config”目录中的“quickntp.ini”文件存在问题。更多详情请参见此处：#customize-servers。

## 免责声明

- 请勿频繁向公共服务器发送请求。NTP 服务器的查询间隔应为 36 小时。

- 此程序会更改 NetworkSystemClock，这可能会导致主机与服务器之间出现时间不同步的情况。请自行承担风险！

[version-badge]: https://img.shields.io/github/v/release/nedex/QuickNTP
[changelog]: ./CHANGELOG.md
[license-badge]: https://img.shields.io/github/license/nedex/QuickNTP
[license]: ./LICENSE
