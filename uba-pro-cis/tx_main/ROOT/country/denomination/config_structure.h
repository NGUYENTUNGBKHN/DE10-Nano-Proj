#ifndef _CONFIG_STRUCTURE_H_
#define _CONFIG_STRUCTURE_H_


enum ChannelType
{
	CHANNEL_R,//0
	CHANNEL_G,//1
	CHANNEL_B,
	CHANNEL_IR1,
	CHANNEL_IR2,
	CHANNEL_RGB,
	//Ducbx --add support more channel --begin
	CHANNEL_FL,
	CHANNEL_T_R,
	CHANNEL_T_G,
	CHANNEL_T_B,
	CHANNEL_T_IR1,
	CHANNEL_T_IR2,
	CHANNEL_MAG,
	CHANNEL_MAG2,
	//Ducbx --add support more channel --end
};

struct Roi
{
	float top;
	float left;
	float bottom;
	float right;
};
enum DenomiAgo
{
	HOG,
	MBLBP
};
struct DenomiHog
{
	u16 orientations;
	u16 pixels_per_cell;
	u16 cells_per_block;
};
struct DenomiMBLBP
{
	u16 blockSize_x;
	u16 blockSize_y;
	u16 cellSize_x;
	u16 cellSize_y;
	u16 gridSize_x;
	u16 gridSize_y;
};
typedef struct
{
	u16 isSymmetric;
	struct Roi rois[4];
	enum DenomiAgo _agorithm;
	enum ChannelType _channel;
	struct DenomiHog _hogParams;
	struct DenomiMBLBP _mblbpParams;

} DenomiConfig;


typedef struct
{
	int len;
	float* data;
}NN_Tensor;

struct NN_PerceptronLayer
{

	NN_Tensor* w;
	NN_Tensor* b;
	NN_Tensor* tensor;
	struct NN_PerceptronLayer* next_layer;
	int act_func_type;
	void (*activation_func)(NN_Tensor* input, NN_Tensor* output);
	int len_feature;
};
typedef struct NN_PerceptronLayer NN_PerceptronLayer;

#endif/*_CONFIG_STRUCTURE_H_*/
