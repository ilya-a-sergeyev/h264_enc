#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "include/vpu_api.h"
#include "include/vpu_global.h"
#include "include/vpu_mem.h"

#define MAX_STREM_LENGTH	1920*1080*3/2

int testEnc(char *FilePath, char *OutFilePath, int32_t enc_width, int32_t enc_height, int32_t enc_framerate, int32_t enc_bitrate)
{
	void *h264encHandle;
	unsigned char* aOutBuffer;
	unsigned long  aOutputLength;
	unsigned char* aInputBuf = NULL;
	unsigned char* aInputTempbuf = NULL;
	unsigned int   aInBufSize;
	unsigned int   InputTimestamp;
	unsigned char *data = NULL;
	EncParams1 mEncPar;
	int aSyncFlag = 0;
	int i = 0,ret;
	int offset = 0;
	int Nalsize = 0;
	FILE *fp_in = NULL;
	FILE *fp_out = NULL;

	VPUMemLinear_t vpu_p_in, vpu_p_out;

	fp_in = fopen(FilePath,"rb");
	if(!fp_in){
		printf("open input file fail");
	}
	fp_out = fopen(OutFilePath,"wb+");
	if(!fp_out)
	{
		printf("open output file fail");
	}

	memset(&mEncPar,0,sizeof(EncParams1));

	mEncPar.width = enc_width;
	mEncPar.height = enc_height;
	mEncPar.framerate = enc_framerate;
	mEncPar.bitRate = enc_bitrate;

	int frame_size = mEncPar.width*mEncPar.height*3/2;

	memset(&vpu_p_in, 0, sizeof(vpu_p_in));
	memset(&vpu_p_out, 0, sizeof(vpu_p_out));
	vpu_p_in.offset = -1;
	vpu_p_out.offset = -1;
	VPUMallocLinear(&vpu_p_in, frame_size);
	VPUMallocLinear(&vpu_p_out, MAX_STREM_LENGTH);

	if (vpu_p_in.offset>0 && vpu_p_out.offset>0) {
		aInputTempbuf = (unsigned char *)VPUMemVirtual(&vpu_p_in);
		aOutBuffer = (unsigned char *)VPUMemVirtual(&vpu_p_out);
	}
	else {
		puts("Linear VPU mem allocation error, userspace buffers will be used.");
		aOutBuffer = (unsigned char*)malloc(MAX_STREM_LENGTH);
		aInputTempbuf = (unsigned char*)malloc(frame_size);
	}

	// get the decoder handle
	printf("get_class_On2AvcEncoder\n");
	h264encHandle =  get_class_On2AvcEncoder();

	// init the decoder handle
	printf("init_class_On2AvcEncoder\n");
	init_class_On2AvcEncoder(h264encHandle,&mEncPar,aOutBuffer,&aOutputLength);

	int nal = 0x01000000;
	fwrite(&nal,1,sizeof(nal),fp_out);
	fwrite(aOutBuffer,1,aOutputLength,fp_out);
	int frames = 0;

	printf("encoding ... ");

	float delta_t = 0;

	while(1)
	{
		aSyncFlag = 0;
		if (fread(aInputTempbuf, 1, frame_size, fp_in) == frame_size) {

			time_t time_start = time(NULL);

			ret = enc_oneframe_class_On2AvcEncoder(h264encHandle,
				aOutBuffer, (unsigned int *)&aOutputLength,
				aInputTempbuf, 0,
				&aInBufSize, &InputTimestamp, &aSyncFlag);

			time_t	time_end = time(NULL);
			delta_t += (int)difftime(time_end, time_start);

			if(aOutputLength > 0 && ret >=0) {
				int nal = 0x01000000;
				fwrite(&nal,1,sizeof(nal),fp_out);
				fwrite(aOutBuffer,1,aOutputLength,fp_out);
				fflush(fp_out);
				frames++;
			}
			aOutputLength = 0;
		}
		else
			break;
	}


	printf(" %i raw frames encoded %.f seconds\n", frames, delta_t);

	if (vpu_p_in.offset>0 && vpu_p_out.offset>0) {
		VPUFreeLinear(&vpu_p_in);
		VPUFreeLinear(&vpu_p_out);
	}
	else {
		free(aOutBuffer);
		free(aInputTempbuf);
	}

	fclose(fp_out);
	fclose(fp_in);
	//enc end deinit the decoder
	printf("deinit_class_On2AvcEncoder\n");
	deinit_class_On2AvcEncoder(h264encHandle);

	// destroy enc handle
	printf("destroy_class_On2AvcEncoder\n");
	destroy_class_On2AvcEncoder(h264encHandle);
	return 0;
}

int main(int argc, const char* const argv[])
{
	if (argc > 6)
		testEnc((char*)argv[1], (char*)argv[2], atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]));
	else
		printf("Usage: h264_enc in_file out_file width height framerate bitrate(bit/s)\n");

	puts("done.");
	return 0;
}
