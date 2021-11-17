
### 1. 编译

本demo是使用的开源项目fdk-aac（[源码下载地址](https://github.com/mstorsjo/fdk-aac)）将PCM数据编码成aac音频文件。由于库文件是在x86\_64架构的主机上面编译生成的，所以仅支持该环境的编译运行，其他CPU架构的可以去访问上面的链接去下载编译。

```bash
$ make
```


### 2. 使用


```bash
$ ./pcm2aac -h
$ ./pcm2aac --help
$ ./pcm2aac -i ./audio/test_8000_16_1.pcm -r 8000 -b 16 -c 1 -o out_8khz_1ch.aac
$ ./pcm2aac --input_pcmfile=./audio/test_44100_16_2.pcm --sample_rate=44100 --sample_bits=16 --channels=2 --output_aacfile=out_44.1khz_2ch.aac
```

其中，fdk-aac对输入的编码数据格式做了些说明：[网页链接](https://wiki.hydrogenaudio.org/index.php?title=Fraunhofer_FDK_AAC)

![aac params](https://i.loli.net/2021/11/16/rx4vtG9jsf1MbQP.png)


### 3. 参考文章

【格式说明】

- [AAC文件格式解析\_cloud 的学习时代-CSDN博客\_aac](https://blog.csdn.net/u013427969/article/details/53091594) 

- [从零开始写一个RTSP服务器（5）RTP传输AAC\_JT同学的博客-CSDN博客](https://blog.csdn.net/weixin_42462202/article/details/99200935) 

- [音频编码格式介绍-AAC - 简书](https://www.jianshu.com/p/515cb88fbc75) 

【编码实现】

- [使用fdkaac编码\_程序人生-CSDN博客](https://blog.csdn.net/lipku/article/details/78564518) 

- [【音频编码】AAC编码之FDK AAC\_CWB的博客-CSDN博客\_fdkaac](https://blog.csdn.net/caiwenbin1101517886/article/details/82082966) 


### 附录（demo目录架构）

```
.
├── audio
│   ├── out_44.1khz_2ch.aac
│   ├── out_8khz_1ch.aac
│   ├── test_44100_16_2.pcm
│   └── test_8000_16_1.pcm
├── docs
│   ├── AAC文件格式解析_cloud 的学习时代-CSDN博客_aac.mhtml
│   ├── 从零开始写一个RTSP服务器（5）RTP传输AAC_JT同学的博客-CSDN博客.mhtml
│   ├── 使用fdkaac编码_程序人生-CSDN博客.mhtml
│   ├── 【音频编码】AAC编码之FDK AAC_CWB的博客-CSDN博客_fdkaac.mhtml
│   └── 音频编码格式介绍-AAC - 简书.mhtml
├── include
│   └── fdk-aac
│       ├── aacdecoder_lib.h
│       ├── aacenc_lib.h
│       ├── FDK_audio.h
│       ├── genericStds.h
│       ├── machine_type.h
│       └── syslib_channelMapDescr.h
├── lib
│   └── libfdk-aac.a
├── main.c
├── Makefile
└── README.md
```

