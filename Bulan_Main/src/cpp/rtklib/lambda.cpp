/*------------------------------------------------------------------------------
* lambda.c : integer ambiguity resolution
*
*          Copyright (C) 2007-2008 by T.TAKASU, All rights reserved.
*
* reference :
*     [1] P.J.G.Teunissen, The least-square ambiguity decorrelation adjustment:
*         a method for fast GPS ambiguity estimation, J.Geodesy, Vol.70, 65-82,
*         1995
*     [2] X.-W.Chang, X.Yang, T.Zhou, MLAMBDA: A modified LAMBDA method for
*         integer least-squares estimation, J.Geodesy, Vol.79, 552-565, 2005
*
*-----------------------------------------------------------------------------*/


#include <math.h>

#include "algebra.hpp"
#include "common.hpp"
#include "trace.hpp"


/* constants/macros ----------------------------------------------------------*/

#define LOOPMAX     10000           /* maximum count of search loop */

const double table1[34][41] =
{
	{0.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000},
	{0.0010,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000},
	{0.0020,0.7796,0.7839,0.8158,0.8053,0.8151,0.8233,0.8312,0.8408,0.8515,0.8621,0.8623,0.8625,0.8710,0.8795,0.8820,0.8874,0.8888,0.8902,0.9190,0.9478,0.8985,0.9057,0.9075,0.9092,0.9129,0.9165,0.9091,0.9203,0.9231,0.9259,0.9348,0.9436,0.9436,0.9436,0.9436,0.9436,0.9436,0.9436,0.9436,0.9436},
	{0.0050,0.5408,0.5432,0.5715,0.5735,0.5889,0.6388,0.6604,0.6848,0.6940,0.7031,0.7096,0.7162,0.7339,0.7516,0.7484,0.7643,0.7771,0.7898,0.8020,0.8142,0.8033,0.8079,0.8127,0.8175,0.8264,0.8353,0.8316,0.8419,0.8463,0.8507,0.8600,0.8692,0.8692,0.8692,0.8692,0.8692,0.8692,0.8692,0.8692,0.8692},
	{0.0100,0.3651,0.3869,0.4131,0.4338,0.4491,0.5084,0.5358,0.5679,0.5814,0.5948,0.6051,0.6153,0.6355,0.6557,0.6762,0.6826,0.6964,0.7102,0.7174,0.7247,0.7315,0.7453,0.7470,0.7488,0.7632,0.7777,0.7764,0.7892,0.7930,0.7967,0.8215,0.8463,0.8463,0.8463,0.8463,0.8463,0.8463,0.8463,0.8463,0.8463},
	{0.0150,0.2687,0.2984,0.3271,0.3608,0.3767,0.4340,0.4647,0.5123,0.5256,0.5390,0.5530,0.5670,0.5863,0.6056,0.6278,0.6369,0.6471,0.6572,0.6692,0.6812,0.7035,0.7161,0.7208,0.7255,0.7372,0.7490,0.7482,0.7644,0.7677,0.7709,0.7989,0.8270,0.8270,0.8270,0.8270,0.8270,0.8270,0.8270,0.8270,0.8270},
	{0.0200,0.2138,0.2428,0.2694,0.3009,0.3301,0.3860,0.4186,0.4797,0.4934,0.5072,0.5183,0.5294,0.5516,0.5737,0.5952,0.6100,0.6168,0.6236,0.6434,0.6632,0.6816,0.6959,0.7016,0.7072,0.7188,0.7303,0.7296,0.7461,0.7497,0.7532,0.7814,0.8095,0.8095,0.8095,0.8095,0.8095,0.8095,0.8095,0.8095,0.8095},
	{0.0250,0.1885,0.2177,0.2418,0.2656,0.2904,0.3493,0.3858,0.4531,0.4672,0.4813,0.4910,0.5007,0.5248,0.5489,0.5714,0.5920,0.5999,0.6079,0.6285,0.6491,0.6645,0.6816,0.6870,0.6925,0.7060,0.7194,0.7128,0.7327,0.7375,0.7422,0.7680,0.7939,0.7939,0.7939,0.7939,0.7939,0.7939,0.7939,0.7939,0.7939},
	{0.0300,0.1709,0.1983,0.2195,0.2418,0.2640,0.3202,0.3595,0.4317,0.4458,0.4599,0.4714,0.4829,0.5079,0.5329,0.5518,0.5765,0.5859,0.5953,0.6166,0.6379,0.6515,0.6697,0.6752,0.6808,0.6961,0.7114,0.6987,0.7240,0.7294,0.7348,0.7574,0.7800,0.7800,0.7800,0.7800,0.7800,0.7800,0.7800,0.7800,0.7800},
	{0.0350,0.1580,0.1833,0.2014,0.2221,0.2439,0.2989,0.3374,0.4147,0.4285,0.4423,0.4552,0.4680,0.4957,0.5234,0.5371,0.5634,0.5740,0.5847,0.6068,0.6288,0.6416,0.6582,0.6650,0.6717,0.6879,0.7042,0.6889,0.7187,0.7234,0.7282,0.7480,0.7677,0.7677,0.7677,0.7677,0.7677,0.7677,0.7677,0.7677,0.7677},
	{0.0400,0.1473,0.1713,0.1867,0.2058,0.2257,0.2826,0.3201,0.4011,0.4145,0.4279,0.4412,0.4546,0.4845,0.5145,0.5276,0.5524,0.5641,0.5758,0.5987,0.6216,0.6339,0.6480,0.6562,0.6644,0.6810,0.6977,0.6847,0.7138,0.7180,0.7222,0.7396,0.7570,0.7570,0.7570,0.7570,0.7570,0.7570,0.7570,0.7570,0.7570},
	{0.0450,0.1382,0.1609,0.1745,0.1921,0.2093,0.2693,0.3063,0.3902,0.4031,0.4160,0.4293,0.4426,0.4745,0.5063,0.5199,0.5435,0.5560,0.5684,0.5921,0.6157,0.6277,0.6395,0.6491,0.6586,0.6752,0.6919,0.6823,0.7093,0.7131,0.7170,0.7323,0.7477,0.7477,0.7477,0.7477,0.7477,0.7477,0.7477,0.7477,0.7477},
	{0.0500,0.1305,0.1508,0.1640,0.1803,0.1947,0.2568,0.2940,0.3808,0.3933,0.4059,0.4191,0.4323,0.4655,0.4987,0.5133,0.5365,0.5495,0.5625,0.5865,0.6106,0.6223,0.6331,0.6433,0.6536,0.6702,0.6868,0.6801,0.7050,0.7087,0.7123,0.7260,0.7397,0.7397,0.7397,0.7397,0.7397,0.7397,0.7397,0.7397,0.7397},
	{0.0550,0.1240,0.1418,0.1544,0.1697,0.1818,0.2447,0.2832,0.3718,0.3844,0.3971,0.4103,0.4236,0.4577,0.4918,0.5075,0.5313,0.5445,0.5577,0.5818,0.6059,0.6177,0.6291,0.6390,0.6489,0.6657,0.6825,0.6780,0.7011,0.7047,0.7083,0.7206,0.7329,0.7329,0.7329,0.7329,0.7329,0.7329,0.7329,0.7329,0.7329},
	{0.0600,0.1182,0.1337,0.1459,0.1597,0.1707,0.2333,0.2742,0.3632,0.3763,0.3893,0.4030,0.4167,0.4511,0.4855,0.5027,0.5273,0.5406,0.5539,0.5777,0.6015,0.6138,0.6262,0.6354,0.6445,0.6617,0.6788,0.6761,0.6975,0.7012,0.7048,0.7160,0.7273,0.7273,0.7273,0.7273,0.7273,0.7273,0.7273,0.7273,0.7273},
	{0.0650,0.1130,0.1264,0.1384,0.1507,0.1613,0.2230,0.2670,0.3551,0.3689,0.3828,0.3972,0.4116,0.4457,0.4797,0.4986,0.5236,0.5373,0.5510,0.5742,0.5974,0.6105,0.6235,0.6320,0.6404,0.6582,0.6759,0.6743,0.6943,0.6981,0.7019,0.7122,0.7226,0.7226,0.7226,0.7226,0.7226,0.7226,0.7226,0.7226,0.7226},
	{0.0700,0.1080,0.1197,0.1318,0.1427,0.1535,0.2141,0.2612,0.3472,0.3622,0.3771,0.3928,0.4085,0.4415,0.4745,0.4952,0.5202,0.5343,0.5484,0.5710,0.5936,0.6076,0.6211,0.6289,0.6366,0.6552,0.6738,0.6727,0.6913,0.6954,0.6995,0.7092,0.7189,0.7189,0.7189,0.7189,0.7189,0.7189,0.7189,0.7189,0.7189},
	{0.0750,0.1030,0.1135,0.1257,0.1357,0.1473,0.2069,0.2559,0.3398,0.3560,0.3723,0.3893,0.4062,0.4381,0.4699,0.4925,0.5170,0.5315,0.5459,0.5680,0.5902,0.6053,0.6188,0.6260,0.6333,0.6528,0.6723,0.6712,0.6886,0.6930,0.6975,0.7068,0.7160,0.7160,0.7160,0.7160,0.7160,0.7160,0.7160,0.7160,0.7160},
	{0.0800,0.0977,0.1074,0.1201,0.1297,0.1420,0.2017,0.2511,0.3326,0.3503,0.3680,0.3860,0.4040,0.4349,0.4658,0.4900,0.5141,0.5289,0.5436,0.5653,0.5871,0.6033,0.6167,0.6236,0.6304,0.6509,0.6713,0.6698,0.6861,0.6911,0.6961,0.7049,0.7138,0.7138,0.7138,0.7138,0.7138,0.7138,0.7138,0.7138,0.7138},
	{0.0850,0.0917,0.1014,0.1147,0.1246,0.1372,0.1976,0.2467,0.3259,0.3449,0.3639,0.3829,0.4019,0.4321,0.4622,0.4877,0.5115,0.5265,0.5414,0.5628,0.5842,0.6016,0.6148,0.6214,0.6280,0.6492,0.6703,0.6685,0.6840,0.6894,0.6949,0.7035,0.7122,0.7122,0.7122,0.7122,0.7122,0.7122,0.7122,0.7122,0.7122},
	{0.0900,0.0849,0.0952,0.1094,0.1196,0.1327,0.1937,0.2427,0.3194,0.3397,0.3599,0.3799,0.3998,0.4295,0.4592,0.4855,0.5091,0.5242,0.5394,0.5605,0.5817,0.6001,0.6131,0.6196,0.6262,0.6478,0.6694,0.6673,0.6821,0.6879,0.6938,0.7024,0.7111,0.7111,0.7111,0.7111,0.7111,0.7111,0.7111,0.7111,0.7111},
	{0.0950,0.0768,0.0886,0.1040,0.1148,0.1286,0.1900,0.2391,0.3133,0.3347,0.3561,0.3770,0.3978,0.4272,0.4566,0.4834,0.5069,0.5222,0.5375,0.5585,0.5794,0.5988,0.6115,0.6182,0.6249,0.6467,0.6685,0.6662,0.6805,0.6866,0.6928,0.7016,0.7104,0.7104,0.7104,0.7104,0.7104,0.7104,0.7104,0.7104,0.7104},
	{0.1000,0.0673,0.0814,0.0982,0.1101,0.1248,0.1865,0.2359,0.3075,0.3300,0.3525,0.3741,0.3958,0.4251,0.4545,0.4815,0.5049,0.5204,0.5358,0.5566,0.5774,0.5975,0.6100,0.6171,0.6242,0.6459,0.6677,0.6652,0.6791,0.6855,0.6919,0.7008,0.7097,0.7097,0.7097,0.7097,0.7097,0.7097,0.7097,0.7097,0.7097},
	{0.1200,0.0000,0.0000,0.0693,0.0935,0.1132,0.1744,0.2261,0.2871,0.3133,0.3395,0.3640,0.3884,0.4188,0.4492,0.4752,0.4991,0.5147,0.5303,0.5508,0.5712,0.5925,0.6051,0.6136,0.6220,0.6433,0.6646,0.6619,0.6759,0.6826,0.6892,0.6983,0.7074,0.7074,0.7074,0.7074,0.7074,0.7074,0.7074,0.7074,0.7074},
	{0.1500,0.0000,0.0000,0.0000,0.0755,0.1035,0.1612,0.2146,0.2642,0.2949,0.3256,0.3522,0.3789,0.4108,0.4427,0.4705,0.4941,0.5102,0.5263,0.5450,0.5638,0.5851,0.5993,0.6092,0.6192,0.6401,0.6611,0.6585,0.6737,0.6804,0.6871,0.6956,0.7041,0.7041,0.7041,0.7041,0.7041,0.7041,0.7041,0.7041,0.7041},
	{0.2000,0.0000,0.0000,0.0000,0.0639,0.0929,0.1480,0.1987,0.2405,0.2763,0.3121,0.3393,0.3665,0.4001,0.4337,0.4670,0.4876,0.5057,0.5238,0.5399,0.5561,0.5758,0.5903,0.6030,0.6158,0.6366,0.6574,0.6534,0.6712,0.6777,0.6842,0.6919,0.6996,0.6996,0.6996,0.6996,0.6996,0.6996,0.6996,0.6996,0.6996},
	{0.2500,0.0000,0.0000,0.0000,0.0563,0.0872,0.1380,0.1864,0.2237,0.2629,0.3021,0.3301,0.3580,0.3923,0.4266,0.4641,0.4824,0.5024,0.5223,0.5374,0.5524,0.5700,0.5831,0.5983,0.6136,0.6349,0.6562,0.6486,0.6696,0.6757,0.6818,0.6889,0.6961,0.6961,0.6961,0.6961,0.6961,0.6961,0.6961,0.6961,0.6961},
	{0.3000,0.0000,0.0000,0.0000,0.0000,0.0846,0.1295,0.1767,0.2112,0.2534,0.2955,0.3241,0.3527,0.3870,0.4213,0.4618,0.4785,0.4998,0.5210,0.5355,0.5499,0.5647,0.5786,0.5953,0.6119,0.6338,0.6557,0.6444,0.6683,0.6741,0.6798,0.6866,0.6934,0.6934,0.6934,0.6934,0.6934,0.6934,0.6934,0.6934,0.6934},
	{0.3500,0.0000,0.0000,0.0000,0.0000,0.0827,0.1228,0.1688,0.2025,0.2472,0.2920,0.3208,0.3496,0.3835,0.4174,0.4600,0.4754,0.4975,0.5196,0.5338,0.5479,0.5601,0.5764,0.5934,0.6104,0.6329,0.6554,0.6410,0.6670,0.6726,0.6782,0.6849,0.6915,0.6915,0.6915,0.6915,0.6915,0.6915,0.6915,0.6915,0.6915},
	{0.4000,0.0000,0.0000,0.0000,0.0000,0.0816,0.1182,0.1615,0.1966,0.2430,0.2894,0.3187,0.3480,0.3814,0.4147,0.4587,0.4730,0.4957,0.5183,0.5320,0.5457,0.5565,0.5746,0.5915,0.6085,0.6318,0.6552,0.6384,0.6658,0.6714,0.6771,0.6837,0.6903,0.6903,0.6903,0.6903,0.6903,0.6903,0.6903,0.6903,0.6903},
	{0.4500,0.0000,0.0000,0.0000,0.0000,0.0816,0.1160,0.1562,0.1921,0.2398,0.2875,0.3172,0.3468,0.3799,0.4129,0.4580,0.4711,0.4941,0.5171,0.5302,0.5434,0.5540,0.5734,0.5899,0.6064,0.6307,0.6550,0.6368,0.6648,0.6706,0.6764,0.6830,0.6896,0.6896,0.6896,0.6896,0.6896,0.6896,0.6896,0.6896,0.6896},
	{0.4990,0.0000,0.0000,0.0000,0.0000,0.0827,0.1159,0.1556,0.1901,0.2384,0.2868,0.3166,0.3464,0.3790,0.4117,0.4576,0.4697,0.4929,0.5161,0.5286,0.5411,0.5530,0.5729,0.5887,0.6045,0.6297,0.6550,0.6364,0.6642,0.6702,0.6762,0.6828,0.6894,0.6894,0.6894,0.6894,0.6894,0.6894,0.6894,0.6894,0.6894},
	{0.5000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000},
	{1.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000}
};
const double table10[31][41] =
{
	{0.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000},
	{0.0100,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000,1.0000},
	{0.0110,0.9635,0.9662,0.9690,0.9714,0.9732,0.9781,0.9777,0.9786,0.9801,0.9816,0.9761,0.9706,0.9762,0.9817,0.9811,0.9842,0.9842,0.9842,0.9868,0.9894,0.9874,0.9876,0.9869,0.9862,0.9891,0.9919,0.9880,0.9922,0.9902,0.9882,0.9920,0.9958,0.9958,0.9958,0.9958,0.9958,0.9958,0.9958,0.9958,0.9958},
	{0.0160,0.7998,0.8149,0.8300,0.8406,0.8515,0.8781,0.8800,0.8837,0.8905,0.8973,0.8969,0.8965,0.9054,0.9142,0.9204,0.9141,0.9221,0.9301,0.9342,0.9382,0.9378,0.9415,0.9422,0.9429,0.9476,0.9523,0.9463,0.9591,0.9553,0.9516,0.9635,0.9755,0.9755,0.9755,0.9755,0.9755,0.9755,0.9755,0.9755,0.9755},
	{0.0210,0.6824,0.7028,0.7232,0.7435,0.7621,0.7951,0.7999,0.8191,0.8289,0.8387,0.8424,0.8461,0.8566,0.8670,0.8779,0.8682,0.8793,0.8904,0.8949,0.8995,0.9033,0.9089,0.9109,0.9129,0.9161,0.9194,0.9196,0.9296,0.9277,0.9257,0.9399,0.9541,0.9541,0.9541,0.9541,0.9541,0.9541,0.9541,0.9541,0.9541},
	{0.0260,0.5879,0.6139,0.6400,0.6721,0.7004,0.7297,0.7405,0.7750,0.7873,0.7996,0.8024,0.8053,0.8202,0.8352,0.8446,0.8439,0.8546,0.8653,0.8704,0.8754,0.8811,0.8841,0.8879,0.8917,0.8953,0.8988,0.8982,0.9055,0.9062,0.9069,0.9197,0.9325,0.9325,0.9325,0.9325,0.9325,0.9325,0.9325,0.9325,0.9325},
	{0.0310,0.5198,0.5514,0.5830,0.6154,0.6500,0.6794,0.7006,0.7422,0.7556,0.7690,0.7723,0.7756,0.7928,0.8099,0.8188,0.8263,0.8351,0.8439,0.8494,0.8549,0.8622,0.8667,0.8711,0.8756,0.8790,0.8824,0.8821,0.8890,0.8910,0.8929,0.9026,0.9123,0.9123,0.9123,0.9123,0.9123,0.9123,0.9123,0.9123,0.9123},
	{0.0360,0.4785,0.5086,0.5388,0.5707,0.5993,0.6428,0.6693,0.7145,0.7290,0.7435,0.7490,0.7546,0.7721,0.7896,0.7973,0.8094,0.8176,0.8258,0.8316,0.8374,0.8459,0.8527,0.8577,0.8627,0.8653,0.8680,0.8696,0.8767,0.8796,0.8824,0.8886,0.8947,0.8947,0.8947,0.8947,0.8947,0.8947,0.8947,0.8947,0.8947},
	{0.0410,0.4392,0.4715,0.5039,0.5337,0.5535,0.6109,0.6434,0.6894,0.7046,0.7198,0.7277,0.7356,0.7542,0.7729,0.7787,0.7941,0.8023,0.8104,0.8165,0.8226,0.8319,0.8408,0.8462,0.8516,0.8535,0.8555,0.8588,0.8670,0.8706,0.8743,0.8779,0.8814,0.8814,0.8814,0.8814,0.8814,0.8814,0.8814,0.8814,0.8814},
	{0.0460,0.4110,0.4431,0.4751,0.5024,0.5223,0.5831,0.6201,0.6669,0.6821,0.6973,0.7075,0.7178,0.7378,0.7578,0.7634,0.7802,0.7888,0.7975,0.8039,0.8103,0.8198,0.8309,0.8365,0.8420,0.8434,0.8448,0.8495,0.8597,0.8632,0.8667,0.8700,0.8732,0.8732,0.8732,0.8732,0.8732,0.8732,0.8732,0.8732,0.8732},
	{0.0510,0.3880,0.4187,0.4494,0.4754,0.4960,0.5589,0.5991,0.6470,0.6617,0.6765,0.6895,0.7024,0.7233,0.7442,0.7507,0.7672,0.7768,0.7865,0.7933,0.8001,0.8094,0.8229,0.8283,0.8338,0.8348,0.8359,0.8412,0.8537,0.8567,0.8597,0.8632,0.8666,0.8666,0.8666,0.8666,0.8666,0.8666,0.8666,0.8666,0.8666},
	{0.0560,0.3673,0.3969,0.4265,0.4515,0.4734,0.5380,0.5803,0.6297,0.6440,0.6582,0.6736,0.6890,0.7104,0.7318,0.7394,0.7561,0.7665,0.7769,0.7842,0.7916,0.8001,0.8166,0.8216,0.8266,0.8276,0.8286,0.8337,0.8480,0.8506,0.8532,0.8570,0.8608,0.8608,0.8608,0.8608,0.8608,0.8608,0.8608,0.8608,0.8608},
	{0.0610,0.3492,0.3777,0.4062,0.4303,0.4535,0.5199,0.5639,0.6151,0.6290,0.6429,0.6598,0.6768,0.6989,0.7210,0.7294,0.7469,0.7578,0.7686,0.7763,0.7840,0.7918,0.8109,0.8154,0.8199,0.8214,0.8229,0.8267,0.8426,0.8449,0.8472,0.8514,0.8556,0.8556,0.8556,0.8556,0.8556,0.8556,0.8556,0.8556,0.8556},
	{0.0660,0.3337,0.3611,0.3885,0.4118,0.4352,0.5042,0.5498,0.6030,0.6170,0.6311,0.6483,0.6655,0.6885,0.7115,0.7207,0.7380,0.7496,0.7612,0.7693,0.7774,0.7845,0.8055,0.8095,0.8136,0.8159,0.8181,0.8208,0.8375,0.8396,0.8417,0.8464,0.8510,0.8510,0.8510,0.8510,0.8510,0.8510,0.8510,0.8510,0.8510},
	{0.0710,0.3209,0.3470,0.3731,0.3962,0.4182,0.4902,0.5370,0.5923,0.6072,0.6221,0.6386,0.6550,0.6792,0.7035,0.7131,0.7294,0.7419,0.7545,0.7630,0.7715,0.7782,0.8003,0.8041,0.8078,0.8107,0.8137,0.8160,0.8327,0.8347,0.8367,0.8419,0.8470,0.8470,0.8470,0.8470,0.8470,0.8470,0.8470,0.8470,0.8470},
	{0.0760,0.3100,0.3350,0.3601,0.3834,0.4028,0.4777,0.5248,0.5828,0.5983,0.6138,0.6296,0.6454,0.6709,0.6965,0.7059,0.7213,0.7348,0.7483,0.7574,0.7664,0.7727,0.7955,0.7989,0.8024,0.8060,0.8097,0.8120,0.8283,0.8303,0.8323,0.8379,0.8436,0.8436,0.8436,0.8436,0.8436,0.8436,0.8436,0.8436,0.8436},
	{0.0810,0.3003,0.3245,0.3488,0.3723,0.3894,0.4660,0.5133,0.5743,0.5901,0.6060,0.6214,0.6368,0.6633,0.6899,0.6992,0.7138,0.7281,0.7424,0.7522,0.7620,0.7681,0.7909,0.7941,0.7974,0.8017,0.8060,0.8084,0.8243,0.8263,0.8284,0.8345,0.8406,0.8406,0.8406,0.8406,0.8406,0.8406,0.8406,0.8406,0.8406},
	{0.0860,0.2912,0.3149,0.3386,0.3621,0.3783,0.4547,0.5026,0.5664,0.5826,0.5987,0.6139,0.6291,0.6564,0.6837,0.6929,0.7069,0.7218,0.7367,0.7474,0.7582,0.7637,0.7865,0.7898,0.7930,0.7978,0.8027,0.8052,0.8206,0.8228,0.8250,0.8315,0.8381,0.8381,0.8381,0.8381,0.8381,0.8381,0.8381,0.8381,0.8381},
	{0.0910,0.2825,0.3055,0.3285,0.3528,0.3685,0.4439,0.4928,0.5591,0.5755,0.5920,0.6070,0.6220,0.6499,0.6778,0.6871,0.7009,0.7160,0.7312,0.7430,0.7548,0.7596,0.7824,0.7858,0.7891,0.7944,0.7997,0.8023,0.8172,0.8196,0.8220,0.8289,0.8359,0.8359,0.8359,0.8359,0.8359,0.8359,0.8359,0.8359,0.8359},
	{0.0960,0.2736,0.2958,0.3179,0.3441,0.3595,0.4337,0.4839,0.5519,0.5688,0.5856,0.6008,0.6160,0.6442,0.6723,0.6818,0.6957,0.7109,0.7262,0.7388,0.7515,0.7558,0.7786,0.7821,0.7857,0.7913,0.7969,0.7998,0.8142,0.8167,0.8192,0.8265,0.8339,0.8339,0.8339,0.8339,0.8339,0.8339,0.8339,0.8339,0.8339},
	{0.1200,0.2294,0.2497,0.2700,0.2989,0.3241,0.3954,0.4541,0.5183,0.5395,0.5607,0.5801,0.5995,0.6252,0.6509,0.6622,0.6812,0.6958,0.7104,0.7239,0.7373,0.7418,0.7635,0.7684,0.7733,0.7802,0.7872,0.7907,0.8051,0.8069,0.8087,0.8171,0.8254,0.8254,0.8254,0.8254,0.8254,0.8254,0.8254,0.8254,0.8254},
	{0.1500,0.1736,0.2027,0.2318,0.2595,0.2893,0.3658,0.4295,0.4860,0.5119,0.5378,0.5621,0.5863,0.6100,0.6338,0.6497,0.6696,0.6850,0.7004,0.7120,0.7236,0.7327,0.7517,0.7576,0.7634,0.7718,0.7802,0.7850,0.7985,0.8008,0.8032,0.8103,0.8175,0.8175,0.8175,0.8175,0.8175,0.8175,0.8175,0.8175,0.8175},
	{0.2000,0.0986,0.1392,0.1797,0.2181,0.2581,0.3283,0.4013,0.4499,0.4799,0.5099,0.5390,0.5680,0.5921,0.6162,0.6369,0.6574,0.6725,0.6876,0.6995,0.7114,0.7234,0.7417,0.7475,0.7534,0.7631,0.7728,0.7797,0.7904,0.7943,0.7983,0.8034,0.8085,0.8085,0.8085,0.8085,0.8085,0.8085,0.8085,0.8085,0.8085},
	{0.2500,0.0394,0.0909,0.1424,0.1873,0.2355,0.2974,0.3791,0.4209,0.4555,0.4900,0.5217,0.5535,0.5799,0.6062,0.6283,0.6476,0.6621,0.6766,0.6909,0.7051,0.7173,0.7305,0.7389,0.7474,0.7578,0.7681,0.7749,0.7851,0.7894,0.7937,0.7975,0.8013,0.8013,0.8013,0.8013,0.8013,0.8013,0.8013,0.8013,0.8013},
	{0.3000,0.0210,0.0711,0.1212,0.1689,0.2136,0.2731,0.3608,0.3978,0.4363,0.4748,0.5090,0.5432,0.5711,0.5989,0.6215,0.6396,0.6545,0.6693,0.6852,0.7011,0.7122,0.7202,0.7315,0.7428,0.7534,0.7641,0.7707,0.7816,0.7858,0.7899,0.7927,0.7954,0.7954,0.7954,0.7954,0.7954,0.7954,0.7954,0.7954,0.7954},
	{0.3500,0.0113,0.0600,0.1086,0.1539,0.1971,0.2541,0.3459,0.3831,0.4243,0.4654,0.5010,0.5367,0.5648,0.5930,0.6164,0.6340,0.6490,0.6639,0.6814,0.6988,0.7084,0.7147,0.7270,0.7393,0.7502,0.7610,0.7668,0.7787,0.7830,0.7873,0.7899,0.7925,0.7925,0.7925,0.7925,0.7925,0.7925,0.7925,0.7925,0.7925},
	{0.4000,0.0112,0.0555,0.0997,0.1425,0.1848,0.2390,0.3341,0.3721,0.4153,0.4586,0.4950,0.5315,0.5593,0.5872,0.6129,0.6314,0.6454,0.6593,0.6782,0.6971,0.7068,0.7139,0.7253,0.7366,0.7479,0.7593,0.7657,0.7765,0.7810,0.7855,0.7889,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924},
	{0.4500,0.0111,0.0520,0.0928,0.1337,0.1751,0.2291,0.3245,0.3613,0.4086,0.4558,0.4914,0.5270,0.5544,0.5818,0.6101,0.6298,0.6430,0.6562,0.6760,0.6958,0.7057,0.7134,0.7240,0.7345,0.7464,0.7584,0.7651,0.7758,0.7799,0.7841,0.7882,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924},
	{0.4990,0.0086,0.0484,0.0881,0.1279,0.1677,0.2232,0.3162,0.3570,0.4060,0.4550,0.4888,0.5227,0.5499,0.5771,0.6086,0.6285,0.6417,0.6550,0.6752,0.6954,0.7053,0.7132,0.7232,0.7331,0.7455,0.7579,0.7649,0.7758,0.7796,0.7834,0.7879,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924,0.7924},
	{0.5000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000},
	{1.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000,0.0000}
};
double erfar(double x)
{
	/* constants */
	double a1 =  0.254829592;
	double a2 = -0.284496736;
	double a3 =  1.421413741;
	double a4 = -1.453152027;
	double a5 =  1.061405429;
	double p  =  0.3275911;
	double t,y;
	/* save the sign of x */
	int sign = 1;
	if (x < 0) sign = -1;
	x = fabs(x);

	/* A&S formula 7.1.26 */
	t = 1.0/(1.0 + p*x);
	y = 1.0 - (((((a5*t + a4)*t) + a3)*t + a2)*t + a1)*t*exp(-x*x);

	return sign*y;
}
/* normal cumulative distribution function -------------------------------------
* args :   double x        I   actual ILS fail-rate for the current geometry
*          double mu       I   mean
*          double sigma    I   sigma
* return : cdf
* ---------------------------------------------------------------------------*/
double normcdfar(double x,double mu,double sigma)
{
	return 0.5 * (1 + erfar((x - mu) / (sigma * sqrt(2.))));
}
/* sort function ---------------------------------------------------------------
* args :   double *Pd      I   vector to be sorted
*          double *index   O sort index
*          int dim         I   dimension
* ---------------------------------------------------------------------------*/
void rsort(double *Pd,int *index,int dim)
{
	int i,j;
	double num,ind;

	/* sort ascending */
	for(i=0;i<dim;i++)
	{
		for(j=0;j+i<dim-1;j++)
		{
			if(Pd[j]>Pd[j+1])
			{
				num = Pd[j];
				ind = index[j];
				Pd[j] = Pd[j+1];
				Pd[j+1] = num;

				index[j] = index[j+1];
				index[j+1] = ind;
			}
		}
	}
}
/* fixed fail-rate to derive the ratio test critical value ---------------------
* args :   double PfILS    I   actual ILS fail-rate for the current geometry
*          int n           I   number of ambiguities
*          double Pf       I   pre-defined fail-rate (0.01 or 0.001)
* return : critical value for ratio test (inversed)
* ---------------------------------------------------------------------------*/
double ffratio(double PfILS,int n,double Pf)
{
	int i,j,dim =34,nt=0,index[34],index1[31];
	double mun[2],Pn[2],mu,diff;
	double Pt[34],Pd[34],Pt1[31],Pd1[31];

	if(Pf==0.001)
	{
		for (i=0;i<dim;i++)
		{
			Pt[i] = table1[i][0];
			Pd[i] = fabs(Pt[i]-PfILS);
			index[i] = i;
		}

		/* sort Pd and update index */
		rsort(Pd,index,dim);
		diff = PfILS-Pt[index[0]];
		if(diff>0)          nt = 1;
		else if (diff<0)	nt = -1;

		Pn[0] = Pt[index[0]];
		Pn[1] = Pt[index[0]+nt];
		mun[0] = table1[index[0]][n];
		mun[1] = table1[index[0]+nt][n];
	}
	else
	{
		dim = 31;
		for (i=0;i<dim;i++)
		{
			Pt1[i] = table10[i][0];
			Pd1[i] = fabs(Pt1[i]-PfILS);
			index1[i] = i;
		}

		/* sort Pd and update index */
		rsort(Pd1,index1,dim);

		diff = PfILS-Pt1[index1[0]];

		if(diff>0)          nt = 1;
		else if (diff<0)	nt = -1;

		Pn[0] = Pt1[index1[0]];
		Pn[1] = Pt1[index1[0]+nt];

		mun[0] = table10[index1[0]][n];
		mun[1] = table10[index1[0]+nt][n];
	}

	j = Pn[0]<Pn[1]?0:1;

	if (Pn[0]-Pn[1]==0)	mu = mun[0];
	else mu = mun[j]+(PfILS-Pn[j])*(mun[0]-mun[1])/(Pn[0]-Pn[1]);

	return mu;
}
/* LD factorization (Q=L'*diag(D)*L) -----------------------------------------*/
int LD(int n, const double *Q, double *L, double *D)
{
	int i,j,k,info=0;
	double a,*A=mat(n,n);

	memcpy(A,Q,sizeof(double)*n*n);
	for (i=n-1;i>=0;i--)
	{
		if ((D[i]=A[i+i*n])<=0.0) {info=-1; break;}
		a=sqrt(D[i]);
		for (j=0;j<=i;j++) L[i+j*n]=A[i+j*n]/a;
		for (j=0;j<=i-1;j++) for (k=0;k<=j;k++) A[j+k*n]-=L[i+k*n]*L[i+j*n];
		for (j=0;j<=i;j++) L[i+j*n]/=L[i+i*n];
	}
	free(A);
	if (info) fprintf(stderr,"%s : LD factorization error\n",__FILE__);
	return info;
}
/* integer gauss transformation ----------------------------------------------*/
void gauss(int n, double *L, double *Z, int i, int j)
{
	int k,mu;

	if ((mu=(int)ROUND(L[i+j*n]))!=0)
	{
		for (k=i;k<n;k++) L[k+n*j]-=(double)mu*L[k+i*n];
		for (k=0;k<n;k++) Z[k+n*j]-=(double)mu*Z[k+i*n];
	}
}
/* permutations --------------------------------------------------------------*/
void perm(int n, double *L, double *D, int j, double del, double *Z)
{
	int k;
	double eta,lam,a0,a1;

	eta=D[j]/del;
	lam=D[j+1]*L[j+1+j*n]/del;
	D[j]=eta*D[j+1]; D[j+1]=del;
	for (k=0;k<=j-1;k++)
	{
		a0=L[j+k*n]; a1=L[j+1+k*n];
		L[j+k*n]=-L[j+1+j*n]*a0+a1;
		L[j+1+k*n]=eta*a0+lam*a1;
	}
	L[j+1+j*n]=lam;
	for (k=j+2;k<n;k++) SWAP(L[k+j*n],L[k+(j+1)*n]);
	for (k=0;k<n;k++) SWAP(Z[k+j*n],Z[k+(j+1)*n]);
}
/* lambda reduction (z=Z'*a, Qz=Z'*Q*Z=L'*diag(D)*L) (ref.[1]) ---------------*/
void reduction(int n, double *L, double *D, double *Z)
{
	int i,j,k;
	double del;

	j=n-2; k=n-2;
	while (j>=0)
	{
		if (j<=k)
			for (i=j+1;i<n;i++)
				gauss(n,L,Z,i,j);
		del=D[j]+L[j+1+j*n]*L[j+1+j*n]*D[j+1];
		if (del+1E-6<D[j+1])
		{ /* compared considering numerical error */
			perm(n,L,D,j,del,Z);
			k=j; j=n-2;
		}
		else j--;
	}
}
/* modified lambda (mlambda) search (ref. [2]) -------------------------------*/
int search(
	int n, 
	int m, 
	const double *L, 
	const double *D,
	const double *zs,
	double *zn, 
	double *s)
{
	int i,j,k,c,nn=0,imax=0;
	double newdist,maxdist=1E99,y;
	double *S=zeros(n,n),*dist=mat(n,1),*zb=mat(n,1),*z=mat(n,1),*step=mat(n,1);

	k=n-1; dist[k]=0.0;
	zb[k]=zs[k];
	z[k]=ROUND(zb[k]); y=zb[k]-z[k]; step[k]=SGN(y);
	for (c=0;c<LOOPMAX;c++)
	{
		newdist=dist[k]+y*y/D[k];
		if (newdist<maxdist)
		{
			if (k!=0)
			{
				dist[--k]=newdist;
				for (i=0;i<=k;i++)
					S[k+i*n]=S[k+1+i*n]+(z[k+1]-zb[k+1])*L[k+1+i*n];
				zb[k]=zs[k]+S[k+k*n];
				z[k]=ROUND(zb[k]);
				y=zb[k]-z[k];
				step[k]=SGN(y);
			}
			else
			{
				if (nn<m)
				{
					if (nn==0||newdist>s[imax])
						imax=nn;
					for (i=0;i<n;i++)
						zn[i+nn*n]=z[i];
					s[nn++]=newdist;
				}
				else
				{
					if (newdist<s[imax])
					{
						for (i=0;i<n;i++)
							zn[i+imax*n]=z[i];
						s[imax]=newdist;
						for (i=imax=0;i<m;i++)
							if (s[imax]<s[i])
								imax=i;
					}
					maxdist=s[imax];
				}
				z[0]+=step[0];
				y=zb[0]-z[0];
				step[0]=-step[0]-SGN(step[0]);
			}
		}
		else
		{
			if (k==n-1)
				break;
			else
			{
				k++;
				z[k]+=step[k]; y=zb[k]-z[k]; step[k]=-step[k]-SGN(step[k]);
			}
		}
	}
	for (i=0;i<m-1;i++)
	{ /* sort by s */
		for (j=i+1;j<m;j++)
		{
			if (s[i]<s[j])
				continue;
			SWAP(s[i],s[j]);
			for (k=0;k<n;k++) SWAP(zn[k+i*n],zn[k+j*n]);
		}
	}
	free(S); free(dist); free(zb); free(z); free(step);

	if (c>=LOOPMAX)
	{
		fprintf(stderr,"%s : search loop count overflow\n",__FILE__);
		return -1;
	}
	return 0;
}
/* lambda/mlambda integer least-square estimation ------------------------------
* integer least-square estimation. reduction is performed by lambda (ref.[1]),
* and search by mlambda (ref.[2]).
* args   : int    n      I  number of float parameters
*          int    m      I  number of fixed solutions
*          double *a     I  float parameters (n x 1)
*          double *Q     I  covariance matrix of float parameters (n x n)
*          double *F     O  fixed solutions (n x m)
*          double *s     O  sum of squared residulas of fixed solutions (1 x m)
*          double Pf     I  pre-defined fail-rate
*          double *Nfix  O  fixed solution (zeros or others in case of cs)
*          int   *index  O  validation status (0-failed,1-successful)
* return : status (0:ok,other:error)
* notes  : matrix stored by column-major order (fortran convension)
*-----------------------------------------------------------------------------*/
int lambda(
	Trace& trace, 
	int n,
	int m, 
	const double *a, 
	const double *Q,
	double *F,
	double *s, 
	double Pf, 
	bool& pass)
{
	int info,i;
	double *L,*D,*Z,*z,*E,ratio,cratio=0,srate=1;

	if (n<=0||m<=0)
		return -1;
	
	L=zeros(n,n); D=mat(n,1); Z=eye(n); z=mat(n,1),E=mat(n,m);

	/* LD factorization */
	if (!(info=LD(n,Q,L,D)))
	{
		/* lambda reduction */
		reduction(n,L,D,Z);

		/* success-rate calculation */
		for (i=0;i<n;i++)
			srate *= (2*normcdfar(0.5/sqrt(D[i]),0,1)-1);

		/* fixed fail-rate to derive critical value */
		cratio=ffratio(1-srate,n,Pf);

		matmul("TN",n,1,n,1,Z,a,0,z); /* z=Z'*a */

		/* mlambda search */
		if (!(info=search(n,m,L,D,z,E,s))) 
		{
			info=solve("T",Z,E,n,m,F); /* F=Z'\E */
		}
		/* ambiguity validation test */
		ratio=s[0]/s[1];
		tracepdeex(2,trace,"  srate = %8.4f %8.2f %8.2f ", srate, 1/cratio, 1/ratio);
		pass = (ratio <= cratio);
	}
	//if (info==-1&&fppde!=nullptr) fprintf(fppde,"Error\n");
	free(L); free(D); free(Z); free(z); free(E);
	return info;
}
