#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>


#include "fdk-aac/aacenc_lib.h"


//#define DEBUG(fmt, args...)
#define DEBUG(fmt, args...) 	printf(fmt, ##args)


void print_usage(const char *process)
{
	printf("sample: \n"
		   "\t %s -h\n"
		   "\t %s --help\n"
		   "\t %s -i ./audio/test_8000_16_1.pcm -r 8000 -b 16 -c 1 -o out_8khz_1ch.aac\n"
		   "\t %s --input_pcmfile=./audio/test_44100_16_2.pcm --sample_rate=44100 --sample_bits=16 --channels=2 --output_aacfile=out_44.1khz_2ch.aac\n",
		   process, process, process, process);
}


int main(int argc, char *argv[])
{
	/* 输入/输出文件 */
	FILE *fpPcm = NULL;
	FILE *fpAac = NULL;
	char pcmFileName[128] = {0};
	char aacFileName[128] = {0};

	/* PCM参数 */
	unsigned int u32PcmSampleRate = 0; // 采样率
	unsigned int u32PcmSampleBits = 0; // 采样位数
	unsigned int u32PcmChannels   = 0; // 声道数

	/* aac编码器 */
	HANDLE_AACENCODER aacEncHandle = NULL; // HANDLE_AACENCODER其实是一个结构体指针
	AACENC_InfoStruct aacEncInfoSt = {0};
	AACENC_ERROR aacErrNum = AACENC_OK; // AACENC_OK:0

	/* 编码相关参数 */
	unsigned int u32PcmInBufBytes = 0; 	// 编码时需要传入的PCM数据大小（字节数）
	unsigned int u32AacOutBufMaxBytes = 0; // 编码后得到一帧aac数据最大的大小（字节数）
	unsigned char *pu8PcmInBuf   = NULL; // 读取pcm并传递进去编码的缓存指针，后面根据编码器传出参数malloc分配
	unsigned char *pu8AacEncBuf  = NULL; // 编码得到的aac缓存，后面根据编码器传出参数malloc分配


	/* 判断输入参数 */
	if(argc == 1)
	{
		print_usage(argv[0]);
		return -1;
	}	

	/* 解析命令行参数 */
	char option = 0;
	int option_index = 0;
	char *short_options = "hi:r:b:c:o:";
	struct option long_options[] =
	{
		{"help",          no_argument,       NULL, 'h'},
		{"input_pcmfile", required_argument, NULL, 'i'},
		{"sample_rate",   required_argument, NULL, 'r'},
		{"sample_bits",   required_argument, NULL, 'b'},
		{"channels",      required_argument, NULL, 'c'},
		{"output_aacfile",required_argument, NULL, 'o'},
		{NULL,            0,                 NULL,  0 },
	};
	while((option = getopt_long_only(argc, argv, short_options, long_options, &option_index)) != -1)
	{
		switch(option)
		{
			case 'h':
				print_usage(argv[0]);
				return 0;
			case 'i':
				strncpy(pcmFileName, optarg, 128);
				break;
			case 'r':
				u32PcmSampleRate = atoi(optarg);
				break;
			case 'c':
				u32PcmChannels = atoi(optarg);
				break;
			case 'b':
				u32PcmSampleBits = atoi(optarg);
				break;
			case 'o':
				strncpy(aacFileName, optarg, 128);
				break;
			defalut:
				printf("Unknown argument!\n");
				break;
		}
 	}
	printf("\n**************************************\n"
		   "input: \n"
		   "\t file name: %s\n"
		   "\t sample rate: %d Hz\n"
		   "\t sample bits: %d bits\n"
		   "\t channels: %d\n"
		   "\t bits per second: %d bps\n"
		   "output: \n"
		   "\t file name: %s\n"
		   "**************************************\n\n",
		   pcmFileName, u32PcmSampleRate, u32PcmSampleBits, u32PcmChannels,
		   u32PcmSampleRate*u32PcmSampleBits*u32PcmChannels, aacFileName);

	/* 先打开输入/输出文件 */
	fpPcm = fopen(pcmFileName, "rb");
	if(fpPcm == NULL)
	{
		char errMsg[128] = {0};
		snprintf(errMsg, 128, "open file(%s) error", pcmFileName);
		perror(errMsg);
		return -1;
	}
	fpAac = fopen(aacFileName, "wb");
	if(fpAac == NULL)
	{
		char errMsg[128] = {0};
		snprintf(errMsg, 128, "open file(%s) error", aacFileName);
		perror(errMsg);
		return -1;
	}

	/* AAC编码 1/8：打开编码器，传出编码器句柄 */
	aacErrNum = aacEncOpen(&aacEncHandle, 0, u32PcmChannels);
	if(aacErrNum != AACENC_OK)
	{
		printf("Open aac encoder error!\n");
		goto error_exit1;
	}

	/* AAC编码 2/8：配置/初始化编码器 */
	// 配置编码器 a/b：设置参数
	aacErrNum  = aacEncoder_SetParam(aacEncHandle, AACENC_AOT, AOT_AAC_LC); 	// Audio object type, 选择输出规格
	aacErrNum |= aacEncoder_SetParam(aacEncHandle, AACENC_SBR_MODE, 1); 		// Spectral Band Replication，是否使能SBR技术，-1:自动配置（默认） 0:关闭 1:开启
	aacErrNum |= aacEncoder_SetParam(aacEncHandle, AACENC_SAMPLERATE, u32PcmSampleRate); // Audio input data sampling rate
	aacErrNum |= aacEncoder_SetParam(aacEncHandle, AACENC_CHANNELMODE, (u32PcmChannels == 1) ? MODE_1 : MODE_2); // 声道模式，还有多种模式，这里只列出2种
	aacErrNum |= aacEncoder_SetParam(aacEncHandle, AACENC_CHANNELORDER, 1); 	// 输入音频数据通道排序方案，0: MPEG频道排序（默认） 1: WAVE文件格式通道排序
	aacErrNum |= aacEncoder_SetParam(aacEncHandle, AACENC_BITRATEMODE, 5); 		// 比特率模式，0:CBR  1~5:VBR（数值越大动态码率越高）
	aacErrNum |= aacEncoder_SetParam(aacEncHandle, AACENC_BITRATE, 128000); 	// 设置比特率大小，只有AACENC_BITRATEMODE设置为静态码率CBR时生效，VBR时忽略
	aacErrNum |= aacEncoder_SetParam(aacEncHandle, AACENC_TRANSMUX, TT_MP4_ADTS); // 传输类型，TT_MP4_ADIF/TT_MP4_ADTS/TT_MP4_LATM_MCP1...
	aacErrNum |= aacEncoder_SetParam(aacEncHandle, AACENC_AFTERBURNER, 1); 		// “加力燃烧室”，提高音质，0:关闭（默认） 1:开启。官方推荐内存和性能足够的话开启
	// 配置编码器 b/b：设置到编码器里面去
	aacErrNum |= aacEncEncode(aacEncHandle, NULL, NULL, NULL, NULL);
	if(aacErrNum != AACENC_OK)
	{
		printf("Configure aac encoder error!\n");
		goto error_exit1;
	}

	/* AAC编码 3/8：获取编码器信息，从这里得到输入PCM缓存应该设置多大、输出最大的aac字节数 */
	aacErrNum = aacEncInfo(aacEncHandle, &aacEncInfoSt);
	if(aacErrNum != AACENC_OK)
	{
		printf("Get aac encoder info error!\n");
		goto error_exit2;
	}

	// 根据上面获得的编码器信息得到一些比较重要的参数
	u32PcmInBufBytes = aacEncInfoSt.frameLength * u32PcmSampleBits/8 * u32PcmChannels;
	u32AacOutBufMaxBytes = aacEncInfoSt.maxOutBufBytes;

	DEBUG("PCM should in bytes: %d \t AAC out max bytes: %d\n", u32PcmInBufBytes, u32AacOutBufMaxBytes);

	/* 根据上面打开编码器信息分配对应大小的缓存 */
	pu8PcmInBuf  = (unsigned char*)malloc(u32PcmInBufBytes);
	pu8AacEncBuf = (unsigned char*)malloc(u32AacOutBufMaxBytes);


	/* 循环从文件中读取PCM数据编码出aac数据写入到文件中 */
	while(1)
	{
		/* aac编码一帧数据用到的参数 */
		AACENC_BufDesc inPcmBufDesc = {0};
		AACENC_BufDesc outAacBufDesc = {0};
		AACENC_InArgs inArgs = {0};
		AACENC_OutArgs outArgs = {0};
		int inIdentifier = IN_AUDIO_DATA;
		int outIdentifier = OUT_BITSTREAM_DATA;
		//int inElsize = 2;
		//int outElsize = 1;
		int inElsize = sizeof(INT_PCM); // 参考aacenc_lib.h:260示例
		int outElsize = sizeof(UCHAR);

		/* AAC编码 4/8：填充编码器需要的参数，包括编码的pcm数据地址，大小等 */
		int s32ReadPcmBytes = fread(pu8PcmInBuf, 1, u32PcmInBufBytes, fpPcm);
		if(s32ReadPcmBytes <= 0)
		{
			break;
		}

		/* AAC编码 5/8：填充编码器需要的参数，包括编码的pcm数据地址，大小等 */
		inPcmBufDesc.numBufs = 1;
		inPcmBufDesc.bufs = (void **)&pu8PcmInBuf;
		inPcmBufDesc.bufferIdentifiers = &inIdentifier;
		inPcmBufDesc.bufSizes = &s32ReadPcmBytes;
		inPcmBufDesc.bufElSizes = &inElsize;

		inArgs.numInSamples = (s32ReadPcmBytes <= 0) ? -1 : s32ReadPcmBytes/2;

		outAacBufDesc.numBufs = 1;
		outAacBufDesc.bufs = (void **)&pu8AacEncBuf;
		outAacBufDesc.bufferIdentifiers = &outIdentifier;
		outAacBufDesc.bufSizes = &u32AacOutBufMaxBytes;
		outAacBufDesc.bufElSizes = &outElsize;

		/* AAC编码 6/8：将pcm编码出aac */
		aacErrNum = aacEncEncode(aacEncHandle, &inPcmBufDesc, &outAacBufDesc, &inArgs, &outArgs);
		if(aacErrNum != AACENC_OK)
		{
			printf("Aac encoder encode error!\n");
			goto error_exit3;
		}

		DEBUG("IN(pcm): [buf bytes: %4d] [channels: %d] [sample cnt per channel: %4d]  ==>   OUT(aac): [encode out bytes: %4d] \n",
			  s32ReadPcmBytes, u32PcmChannels, inArgs.numInSamples/u32PcmChannels, outArgs.numOutBytes);

		if(outArgs.numOutBytes == 0)
		{
			continue;
		}

		/* AAC编码 7/8：将编码出的aac数据写入文件 */
		fwrite(pu8AacEncBuf, 1, outArgs.numOutBytes, fpAac);
	}

	printf("\n\033[32m%s ==> %s Success!\033[0m\n", pcmFileName, aacFileName);

error_exit3:

	/* 记得释放内存 */
	free(pu8PcmInBuf);
	free(pu8AacEncBuf);

error_exit2:

	/* AAC编码 8/8：关闭编码器 */
	aacEncClose(&aacEncHandle);

error_exit1:

	fclose(fpPcm);
	fclose(fpAac);

	return 0;
}

