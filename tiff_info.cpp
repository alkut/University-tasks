#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

const char PhotoType[5][30] = { "WhiteIsZero\0","BlackIsZero\0","RGB\0","Palette color\0","Transparency Mask\0" };

#define TIFF_SUCCESS	0  
#define NOT_TIFF		-1
#define READ_ERR        -2

uint32_t shift_of_date = -1;

struct tiff_tag
{
	uint16_t TagId, DataType;
	uint32_t  DataOffSet, DataCount;
};

int read_tiff_tag(FILE* f, tiff_tag* tag)
{
	int res=0;
	res += fread(&tag->TagId, sizeof(uint16_t), 1, f);
	res += fread(&tag->DataType, sizeof(uint16_t), 1, f);
	res += fread(&tag->DataCount, sizeof(uint32_t), 1, f);
	res += fread(&tag->DataOffSet, sizeof(uint32_t), 1, f);
	return res-3;
}

void show_tag_info(tiff_tag& tag, int& res, FILE *f, uint8_t shift_to_next)
{
	switch (tag.TagId)
	{
	case 277:

		printf("SamplesPerPixel is %d\n", tag.DataOffSet);
		break;
	case 256:
		if (tag.DataCount * ((tag.DataType == 3) ? 2 : 4) <= 4)
		{
			printf("ImageWidth is %d\n", tag.DataOffSet);
			break;
		}
		fseek(f, tag.DataOffSet, SEEK_SET);
		res = fread(&tag.DataOffSet, 4, 1, f);
		printf("ImageWidth is %d\n", tag.DataOffSet);
		res = fseek(f, shift_to_next, SEEK_SET);
		break;
	case 257:
		if (tag.DataCount * ((tag.DataType == 3) ? 2 : 4) <= 4)
		{
			printf("ImageHeight is %d\n", tag.DataOffSet);
			break;
		}
		fseek(f, tag.DataOffSet, SEEK_CUR);
		res = fread(&tag.DataOffSet, 4, 1, f);
		printf("ImageHeight is %d\n", tag.DataOffSet);
		res = fseek(f, shift_to_next, SEEK_SET);
		break;
	case 278:
		if (tag.DataCount * ((tag.DataType == 3) ? 2 : 4) <= 4)
		{
			printf("RowsPerStrip is %d\n", tag.DataOffSet);
			break;
		}
		fseek(f, tag.DataOffSet, SEEK_CUR);
		res = fread(&tag.DataOffSet, 4, 1, f);
		printf("RowsPerStrip is %d\n", tag.DataOffSet);
		res = fseek(f, shift_to_next, SEEK_SET);
		break;
	case 262:
		if (tag.DataCount * ((tag.DataType == 3) ? 2 : 4) <= 4)
		{
			printf("PhotoMetricInterpretation is %s\n", PhotoType[tag.DataOffSet]);
			break;
		}
		fseek(f, tag.DataOffSet, SEEK_CUR);
		res = fread(&tag.DataOffSet, 4, 1, f);
		printf("PhotoMetricInterpretation is %s\n", PhotoType[tag.DataOffSet]);
		res = fseek(f, shift_to_next, SEEK_SET);
		break;
	case 306:
		if (tag.DataCount * ((tag.DataType == 3) ? 2 : 4) <= 4)
		{
			printf("Date of creation is %d\n", tag.DataOffSet);
			break;
		}
		fseek(f, tag.DataOffSet, SEEK_SET);
		shift_of_date = tag.DataOffSet;
		char data[20];
		res = fread(data, 1, 20, f);
		printf("Date of creation is ");
		for (int i = 0; i < 20; ++i)
		{
			printf("%c", data[i]);
		}
		printf("\n");
		res = fseek(f, shift_to_next, SEEK_SET);
		break;
	default:
		break;
	}
}

int check_version(FILE* f)
{
	fseek(f, 0, SEEK_SET);
	int res;
	uint8_t vers[2];
	for (int i = 0; i < 2; i++)
	{
		res = fread(&vers[i], sizeof(vers[i]), 1, f);
	}
	if (vers[0] == vers[1])
	{
		if (vers[0] == 73)
		{
			//printf("order is II\n");
		}
		else if (vers[0] == 77)
		{
			//printf("order is MM\n");
		}
		else
		{
			return NOT_TIFF;
		}
	}
	else
	{
		return NOT_TIFF;
	}
	uint8_t dd[2];
	for (int i = 0; i < 2; i++)
	{
		res = fread(&dd[i], sizeof(dd[i]), 1, f);
	}
	printf("\n");
	if (vers[0] == 73)
	{
		if (!(dd[0] == 42 && dd[1] == 0))
			return NOT_TIFF;
	}
	if (vers[0] == 77)
	{
		if (!(dd[1] == 42 && dd[0] == 0))
			return NOT_TIFF;
	}
	return TIFF_SUCCESS;
}


int read_TIFF_info(FILE* f)
{
	//чтение хедера, состоит из проверки файла и нахождение отступа первой группы тэгов
	int res = check_version(f);
	if (res != TIFF_SUCCESS)
	{
		return res;
	}
	uint32_t shift;
	fread(&shift, sizeof(shift), 1, f);
	//как только значение отступа на следующую группу тегов равно 0, это значит, что текущая-последняя
	while (shift!= 0) {
		fseek(f, shift, SEEK_SET);
		uint16_t NumTag;
		fread(&NumTag, 2, 1, f);
		tiff_tag tag;
		for (int i = 0; i < NumTag; i++)
		{
			res = read_tiff_tag(f, &tag);
			if (res != 1)
			{
				return NOT_TIFF;
			}
			show_tag_info(tag, res, f, shift + 2 + 12 * (i + 1));
		}
		fread(&shift, sizeof(shift), 1, f);
	}

	return TIFF_SUCCESS;
}

uint8_t* copy_file(FILE* in, int* size_)
{
	fseek(in, 0, SEEK_END);
	size_t size_file = ftell(in);
	*size_ = size_file;
	uint8_t* res = (uint8_t*)malloc(size_file * sizeof(uint8_t));
	fseek(in, 0, SEEK_SET);
	fread(res, sizeof(uint8_t), size_file, in);
	return res;
}

void change_date(const char* filename,const char* new_date)
{
	printf("shift is %d\n", shift_of_date);
	if (shift_of_date == -1)
	{
		printf("No date metadata tag\n");
		return;
	}
	FILE* tmmp;
	fopen_s(&tmmp,filename, "rb");
	int size;
	uint8_t *buffer=copy_file(tmmp,&size);
	for (int i = shift_of_date; i < shift_of_date + 20; ++i)
	{
		buffer[i] = new_date[i - shift_of_date];
	}
	fclose(tmmp);
	FILE* f;
	fopen_s(&f, filename, "wb");
	fwrite(buffer, sizeof(uint8_t),size, f);
	free(buffer);
	fclose(f);
}


int main()
{
	FILE* f;
	errno_t err;
	const char* filename = "C:/Users/User/source/repos/tiff_info/tiff_info/sample.tiff";
	err = fopen_s(&f, filename, "rb");
	if (err != 0)
	{
		printf("Error openning file\n");
		return err;
	}
	int ret;
	printf("before changing \n");
	if ((ret = read_TIFF_info(f)) < 0)
	{
		printf("Error reading tiff file: %d\n", ret);
		return ret;
	};
	fclose(f);
	const char* new_date = "02-10-2021 16:32:42";
	change_date(filename, new_date);
	printf("\nafter changing \n");
	err = fopen_s(&f, filename, "rb");
	fseek(f, 0, SEEK_SET);
	if (err != 0)
	{
		printf("Error openning file\n");
		return err;
	}
	if ((ret = read_TIFF_info(f)) < 0)
	{
		printf("Error reading tiff file: %d\n", ret);
		return ret;
	};
	fclose(f);
	return 0;
}
