#include "StdInc.h"
#include "Tool.h"

void writeRawfile(zoneInfo_t* info, ZStream* buf, Rawfile* data)
{
	Rawfile* dest = (Rawfile*)buf->at();
	buf->write(data, sizeof(Rawfile), 1);
	buf->pushStream(ZSTREAM_VIRTUAL);

	unsigned int writeLen = data->sizeCompressed;
	if (writeLen == 0) writeLen = data->sizeUnCompressed + 1;

	buf->write(data->name, strlen(data->name) + 1, 1);
	dest->name = (const char*)-1;

	if (data->compressedData)
	{
		//buf->align(ALIGN_TO_0);
		buf->write(data->compressedData, writeLen, 1);
		dest->compressedData = (char*)-1;
	}

	buf->popStream();
}

void * addRawfile(zoneInfo_t* info, const char* name, char* data, int dataLen)
{
	if(dataLen < 0) return data; // no fixups needed here

	z_stream strm;
	memset(&strm, 0, sizeof(z_stream));
	char* dest = new char[dataLen*2];

	strm.next_out = (Bytef*)dest;
	strm.next_in = (Bytef*)data;
	strm.avail_out = dataLen*2;
	strm.avail_in = dataLen;

	if(deflateInit(&strm, Z_BEST_COMPRESSION) != Z_OK) { Com_Error(false, "Failed to compress zlib buffer!"); return NULL; }
	if(deflate(&strm, Z_FINISH) != Z_STREAM_END) { Com_Error(false, "Failed to compress zlib buffer!"); return NULL; }
	if(deflateEnd(&strm) != Z_OK) { Com_Error(false, "Failed to compress zlib buffer!"); return NULL; }

	Rawfile* ret = new Rawfile;
	ret->name = strdup(name);
	ret->sizeCompressed = strm.total_out;
	ret->sizeUnCompressed = dataLen;
	ret->compressedData = dest;

	return ret;
}