#pragma once
#ifndef DITHER_KALLEBACH_DATA_H
#define DITHER_KALLEBACH_DATA_H

const int dither_arrays[4][32][32] = {{{172,  40, 126, 240, 213,  72,  49, 171,  94, 207,  69, 153, 251,  17,  96, 166, 222, 106,  47, 158, 224,  14, 138, 239,  36, 192, 168,  99, 116, 200,  37, 237},
                                       { 84,  23, 193,  56,  12, 140, 226, 146, 126, 242,  25,  53, 194, 118, 186,  58,  24, 253, 203,  92,  60, 114, 194,  55, 153,  82, 228,  11, 216, 253,  83,  74},
                                       {232, 147, 101, 166,  86, 203, 189,  64,   8, 179, 107,  81, 139,   1, 237, 146, 133,  75,   7, 128, 191, 244,  97, 211,  71,  23, 129, 144,  43, 150,   6, 124},
                                       {210, 114, 248,   1, 113,  31, 255, 104,  41, 220, 199, 150, 226,  41, 206, 102,  33, 177, 239, 169,  27,  42, 123,   3, 159, 248, 197,  52,  95, 182, 164, 220},
                                       { 76,  16, 217, 186, 120, 230,  21,  77, 212, 115,  16,  30, 171, 123,  86, 163, 197,  95, 215,  80, 107, 149, 183, 223, 110, 177,  65, 205, 236,  24, 108,  61},
                                       {150,  53,  93, 206,  43, 131, 180, 162, 151, 133, 191,  93,  58, 215,  46, 225,  45, 119,  16, 161,  17, 252,  64,  46, 137,  31,  16, 120, 155, 134,  34, 201},
                                       {195,  29, 160,  68, 144,  58, 236,  96,   1,  51, 240, 176, 232, 143,  73, 136, 173,  64, 129, 196, 228, 175,  91, 207, 234, 102, 169,  87,  56, 225, 243,  79},
                                       {127, 247, 175, 225, 107,  12, 170, 200,  68, 250,  38,  83, 112,  10, 184,  28, 248, 208,   2,  34,  53, 142, 109,  10,  77, 190, 255, 210,   4, 172, 100, 146},
                                       {  7,  98,  78,  39, 189, 243,  85,  30, 139, 120, 123,  21, 158, 211,  99, 122, 162, 113, 185,  75,  98, 238,  20, 181, 127,  25, 143,  71, 111,  47, 187,  11},
                                       {219, 166, 118,  23, 133,  69, 222, 108, 239, 183, 207, 224,  72, 199,  37,  63,  85, 223,  57, 152, 213, 165,  52, 221, 154,  45, 200, 162,  92, 216, 251, 130},
                                       { 63, 141, 229, 195,  55, 154, 100,  44, 165,  10, 148,  49, 171, 112, 254, 235,  18, 105, 243,   0, 201, 122,  66, 249, 116,  62, 227,  15, 135,  32,  67, 193},
                                       {185,  48,  75, 255, 176, 214,  18, 184,  81,  59,  97,  29, 132,   2, 161, 147, 138, 177,  10, 152,  29,  83, 141,   4,  96,  39, 239, 105, 180, 233, 157,  22},
                                       { 12, 159, 111,  88,   1, 116,  66, 203, 128, 231, 197, 215, 247,  91,  79,  50, 187, 115,  92, 215, 230,  48, 176, 194, 216, 188, 122,  78,  54, 147, 106,  86},
                                       {128, 240, 210,  30, 234, 168, 143, 250, 160,  17, 105, 154, 179,  66, 204,  33, 240,  71,  39, 182, 131, 106, 245,  34,  73, 148, 167, 253,   7, 202,  42, 242},
                                       {198,  38, 101, 150, 130,  46,  93,  23,  35,  57, 136,  85,   0, 124, 136, 221, 102, 249, 148,  70,  81,  15, 204,  88, 125,  60,  28, 208,  69, 119, 176, 217},
                                       { 85, 167,  20,  57, 252, 190, 221, 109, 188, 201, 245,  45, 192, 226,   9, 172,  24, 156,   4, 212, 164,  27, 154, 170, 241,  13, 223, 130,  97, 231,  18,  60},
                                       {108, 218, 181, 117, 158,  11,  79, 153, 233,   2,  73, 168, 209,  54, 145, 117,  59,  86, 193, 127, 235,  56,  98, 220, 112,  48, 180, 157,  36, 188, 163, 135},
                                       {  6,  61, 246,  89, 204, 137,  38,  97, 174, 144,  28, 114,  94, 103, 184,  71, 241, 225,  36, 125,  50, 252, 188,  40, 137, 194,  91, 249, 107,   3,  83, 205},
                                       {229, 139,  70,  43,  27, 186, 241,  68, 122, 222,  46, 135, 236,  19, 161, 222,  26, 134,  76, 195, 105, 141,  80,   6,  67, 238,  22, 143,  50, 216, 241,  28},
                                       {175,  95, 167, 126, 214, 102, 229,  17,  53, 254, 181, 208,  62,  37, 199, 126,  47, 149, 173,  14, 228,  21, 213, 198, 146, 121, 206,  55, 174, 117, 152,  89},
                                       { 51,  13, 202, 148,  59,  80, 180, 157, 192,  76,   9, 152,  78, 244,   5, 174, 212,  95,  63, 237, 117, 159,  60, 178, 103,  43,  76,  32, 224,  64, 191, 140},
                                       {161, 119, 238,  20, 253,  45, 113, 141,  32, 110, 127, 202, 170, 140, 115,  82,  34, 251, 183,   6,  94,   0, 132, 245,  14, 231, 182, 156, 131,  40,  18, 247},
                                       { 65, 211,  36, 198, 219, 134,   3, 223, 246, 163,  88,  40,  25,  90, 252,  52, 160, 108, 137, 218, 153, 202, 111,  35, 165, 124, 217,  92, 255, 168,  73, 110},
                                       {192,  82, 106,  90, 155,  72, 209,  61,  19, 234,  55, 226, 211, 132, 190, 171, 233,  19,  42,  54, 166,  88, 206,  72,  51, 196,  82,  26, 103, 235, 205,  48},
                                       {236,   7, 142, 243,  52, 121, 169,  99, 187, 145, 104, 173,  65,  13,  69,   8, 100, 204, 124, 227, 210,  26, 191, 237, 140,   3, 151,  67,   9, 186,  84, 156},
                                       { 44, 131, 200,  27, 185, 232,  11, 203,  90, 129,  31, 196, 118, 136, 238, 145, 218,  30,  91,  67, 104,  12, 121, 169,  98, 250, 198, 213, 164, 138,  33, 218},
                                       { 74, 177, 214,  84, 160,  39, 112, 249,  49,   5, 214, 158,  24, 184,  80, 244,  58, 157, 175, 235, 142,  81,  62,  38, 228,  14, 115,  41, 232,  59,  20, 118},
                                       {227, 114,  19, 254, 101,  68, 135, 149, 179,  70, 242,  87,  56, 201,  37, 120,   0, 207, 113, 189,  49, 182, 248, 156,  87,  29, 178,  78, 144,  94, 250, 174},
                                       {151,  32,  96, 189,   5, 208, 230,  22, 164,  99, 190,  47, 147, 224, 178, 163, 103,  75,  21, 133,   4, 221, 199, 134, 109, 165, 242, 129,   2, 209, 196,  74},
                                       {178,  61, 145, 125, 233,  57, 195,  79, 220, 119, 254,  15, 110,  93,  50,  66, 230, 193,  41,  90, 212, 101,  22,  54, 205,  70, 219, 104,  51, 151, 111,  35},
                                       { 77, 247, 219,  44, 173,  89, 109,   8,  35, 138,  42, 170, 209, 244, 128,   5, 217, 139, 245, 167,  63, 149, 123, 179,  44,   8, 187,  26, 172, 246,  15, 227},
                                       {197,   9, 100, 162,  25, 130, 246, 155, 185,  65, 231, 121,  74,  33, 183, 155,  84,  13, 116, 181,  31, 229,  77, 251, 142,  89, 234, 125,  62, 159,  87, 132}},
                                      {{185,  22, 140, 215,  92, 182,  66, 125, 222,  95,   7, 193, 153, 108,  24, 174, 236,  57,  98, 205,  49, 136, 169,  33, 110, 241,  50, 173,  78, 221,  44, 157},
                                       {105,  57, 241, 201,  15,  34, 251, 199,  79, 163, 240,  54,  46, 210,  77, 118, 140,  28, 161, 251,   8,  92, 216,  63, 192,  17,  96, 212, 133, 114,  64, 250},
                                       {168,  38, 130,  72, 117, 165,  49, 141,  22, 175, 114, 134, 227,  88, 196, 250,  43, 222, 186, 108, 124, 197, 154,  13, 150, 131, 239, 165,  33, 202,  11, 101},
                                       {206, 231,   7, 195, 149, 225, 107,   5,  55, 211,  67,  30, 201,  17, 160,   6, 101,  64,  87, 143,  40,  59, 115, 227, 207,  73,   0,   5,  91, 162, 187, 142},
                                       { 17,  86, 254, 101, 176, 237,  88, 187, 243,  99, 149, 255, 124,  93,  69, 248, 171, 203,  19, 213, 190, 245,  75,  25,  83, 181, 117, 144, 218,  82, 121,  26},
                                       {139, 184,  50,  62,  27,  42, 129, 158, 219, 122,  14, 180,  41, 218, 187, 135, 117, 237,  48, 132, 158,   3, 178, 138,  76, 244, 209,  63, 225,   8, 179, 238},
                                       { 71, 118, 155, 217,  83,  18, 203,  70,  27,  61, 233, 165,  80, 143,  26,  57,  35, 152,  96, 233,  69,  31, 223, 104, 200,  12, 127,  41, 191, 109,  56,  46},
                                       {198, 232,   5, 135, 171, 146, 223, 113, 180, 138,  45, 193,  51, 111, 225, 194,  11,  81, 163,   0, 122, 104, 169, 237,  22, 155, 173,  89,  21, 135, 158, 212},
                                       { 29, 165, 245, 105, 191, 252,  56,   9, 230,  85, 106, 214,   4, 156, 241, 120, 250, 149, 185, 228, 199,  53, 147,  65,  48, 249,  97, 184, 253, 226,  67,  90},
                                       { 58,  90,  69,  23, 123,  36,  76, 162, 209,  32, 248, 173, 129,  89,  16,  75, 177,  23,  87,  44,  95,  37, 211, 182, 116,  51, 141,  33,  77, 167,  36, 148},
                                       {226, 132, 179,  45, 234,  95, 184, 109, 127, 150,  20,  99, 189,  38, 171, 130, 210,  62, 141, 238, 136, 244,  10,  80, 131, 220,   2, 206, 123,  14, 194, 239},
                                       {116, 209, 150,  99, 202, 137, 244,   4, 195, 240,  70, 141, 228, 205,  55, 219, 105,   3, 100, 170, 156, 115, 196, 178,  24, 213, 164, 102,  59, 231, 112,  74},
                                       {170,   8, 254,  52, 167,  13, 153,  90,  46, 212,  42, 116,   9, 162,  65, 151,  35, 201, 224,  17,  30, 226,  98,  58, 239,  42, 150,  85, 204, 174, 133,  25},
                                       { 76,  34,  82, 220,  62,  29, 207,  67, 121, 172,  58, 183,  97, 255,  28, 191, 233,  79, 180, 125,  47,  86,   0, 145, 112,  72, 127, 252,   7, 157,  50, 215},
                                       {140, 195, 158, 122, 238, 189,  84, 253, 225, 159,  23, 234, 131,  83, 114,  19, 143,  73, 112, 246, 198, 160, 187, 232, 169,  12, 181,  30, 224,  40,  95, 249},
                                       {246,  91, 110,  43, 145, 106, 178,  51, 147,  11, 108, 197, 216,  44, 177, 243,  92, 166,   9,  60,  70, 104,  37, 120, 242,  94, 194, 137, 109,  67, 183, 145},
                                       { 14, 185, 212,   5, 229,  20, 128,  36,  77,  93, 247,  55, 153,   3, 208, 123, 196,  52, 217, 149, 229, 207,  74,   2,  52, 203, 231,  61, 205, 166, 120,  27},
                                       { 64, 103, 134, 198, 172,  73, 164, 221, 200, 176, 118,  33,  69, 139,  81, 103,  31, 133, 183,  42,  21, 176, 164, 217, 152,  22,  80,  44, 248,  16,  88, 215},
                                       {125, 236,  30,  45, 247,  97,  59, 241, 136, 211,  18, 188, 230, 252, 157, 220,  15, 235,  99,  82, 250, 126,  66,  91, 111, 132, 188, 100, 126, 191, 235, 155},
                                       {161, 181, 216,  78, 154, 119,  28, 107,  43,  63, 161, 128,  91, 170,  10,  61, 201, 245, 173, 117,  56,  31, 199,   9, 254, 208, 161,   1, 143,  55,  35,  72},
                                       {  7,  93,  56, 111,  12, 210, 183, 147,   4,  83, 236, 104,  50,  40, 113, 146, 128,  47,   2, 206, 139, 239, 105, 192,  49,  78,  31, 222, 237, 170, 113, 132},
                                       {229, 202, 144, 255, 194,  84, 226, 252, 169, 217, 192,  26, 178, 205, 235,  77,  94, 166,  68, 154, 220,  59, 182, 119, 228,  96, 177,  88,  68, 200,  20, 221},
                                       { 80, 122,  36, 166,  68,  20,  52, 115,  75,  34, 138, 224,  70, 156, 190, 215,  21, 228, 109,  38,  87,  16, 235,   1, 148,  19, 136, 247, 151,  43,  79, 249},
                                       {175,  24, 188, 130, 238, 103, 139, 202, 126,  94,  19, 151, 120,  86,   2,  62, 134, 186, 197, 248, 174, 124,  73, 114, 167, 214,  66,  53,   8, 184,  98, 144},
                                       {100, 221,  61,   4, 213, 179,  41, 233,  10, 175, 246, 208,  39,  54, 172, 255,  32, 118,  51, 146,  25, 190, 209,  39, 243,  25, 107, 192, 164, 251,  60,  29},
                                       {242, 140, 112, 157,  90,  29, 152, 249,  57, 160,  98, 130, 243, 107, 198, 100, 142, 223,  81,  10, 102, 242, 137,  78, 156, 127, 229,  37, 204, 121, 218, 175},
                                       { 11,  53, 206, 245,  74, 123, 219, 110,  84, 196,  26,  64, 185,  92, 159,  18, 240,  41, 177, 219, 163,  32,  85,  48, 172,  58, 145,  75,  85, 133,  49, 111},
                                       {189,  86,  39, 168, 195,  15,  65, 186, 137,  46, 227, 167,  14, 232,  48, 125, 189, 152,  71,  94, 128,  63, 232, 181, 216,   0, 101, 236, 154,  68,  16, 162},
                                       {147, 234,  23, 135, 102,  47, 231, 171,  39, 213, 102, 148,  34, 218,  76, 207,  60,   1, 110, 200, 253,  15, 153,  40, 106, 197,  13, 174,   1, 240, 188, 211},
                                       { 66, 121, 204, 180, 253, 151,  89, 129,  71, 254, 119,  81, 179, 113, 138,  27,  84, 230, 214,  37, 182, 116, 142, 247,  71, 134, 222, 119, 203, 106,  35,  87},
                                       {193,  45,  79,   6,  60,  32, 242, 210,   3, 190,  54, 246, 204,   6, 168, 155, 244, 129, 146, 103,  53,  24, 208,   6,  89, 186,  28,  93, 251,  72, 126, 142},
                                       {214,  97, 160, 224, 115, 176, 108,  12, 144, 159,  18, 131,  65, 223,  96,  38, 199,  74,  13, 193, 159, 227,  82, 124, 163, 234,  47, 148,  21, 168, 230,  54}},
                                      {{175,  21, 137, 238,  18, 201, 170,  68, 165, 240, 100,  31, 126, 173,  22, 210,  47, 153, 223,  19,  44, 239, 169,  60, 114, 250,  14, 175,  46, 133, 185, 214},
                                       { 54, 109, 209,  88, 148,  38,  83, 112,  23, 213,  74, 152, 194, 231, 101, 248, 137, 116,  62, 199, 134, 150, 100,  26, 215, 144, 107, 196, 228,  67,   8,  63},
                                       {196,  73,  33, 228, 120, 191, 219,  35, 130, 181,  50, 252,  61,  67,  41,  80, 177,  79,  33, 253,  90,   7, 232, 189,  94,  39,  77, 159,  97, 147, 249,  90},
                                       {247, 183, 158,  48,   7, 100, 254, 142,  56, 225, 123,   8, 145, 110, 202, 166,   2, 236, 185, 159, 122, 175,  72, 209, 129, 200,  52, 244,  23,  32, 109, 157},
                                       { 92, 143, 107, 131, 243, 166,  76, 195, 155,   2, 207,  88, 189, 240,  25, 152, 217,  82,  17, 106, 221,  57, 110,  19, 163,   4, 218, 184, 137, 191, 223, 206},
                                       { 13,  42, 235,  64, 203,  28,  15,  91, 236, 103, 136, 174,  14, 118,  54, 182,  69, 126, 141,  48, 193,  36, 248,  86, 142, 114, 236,  80,  95,  49, 128,  59},
                                       {174, 224, 193,  83, 116, 179, 138, 251,  47,  67,  25, 247,  96,  80, 209,  94,   0, 198, 171, 232,  87, 152, 227, 204, 180,  69,  46, 119, 170, 253, 106,  14},
                                       {123,  71, 149, 164,  38, 233,  52, 124, 168, 211, 112, 195, 231, 162, 142, 222, 111,   9, 242,  75, 131,   3,  99, 124,  12,  31, 155, 211,   6,  39, 180, 239},
                                       {207,  25,  55,  97,   2, 201, 105, 188,  11,  84, 151,  59,   1,  49,  35, 226, 179,  58,  45, 162,  29, 187,  60, 169, 217, 251, 197,  62, 229, 117,  74, 154},
                                       {104, 245, 221, 133, 194, 156,  78, 144, 241,  32, 245, 184, 129, 101, 176,  19,  81, 153, 220,  98, 210, 253, 113,  25,  54, 144, 108,  23, 186, 140, 214,  93},
                                       { 45, 182,  15, 171, 216,  65,  21,  43, 227,  96,  41,  73, 145, 215,  29, 237, 134, 206, 119,  17,  70, 156, 140, 241, 192,  78, 130,  69, 163,  48, 202,  27},
                                       {144,  81, 117,  33,  90, 255, 119, 205, 135, 161,  16, 197, 222, 114,  55, 168,  66,   3, 195, 147,  34, 229,  50, 121,  40, 226,   5, 245,  89, 235,  13, 134},
                                       {208, 167, 240,  51, 129, 178, 150,  58, 187,  70, 202, 107,   0,   9, 121, 192, 249, 108,  88, 239, 185,  85,  10, 181, 102, 206, 173, 151,  36,  83, 250, 172},
                                       {  2,  63, 142, 213,  11,  71,  28,  99, 249,   1, 125, 235, 158, 183,  87, 228,  42, 160,  27, 171, 124, 219, 136,  63, 160,  26, 110, 220, 196, 122, 102,  59},
                                       {133,  95, 230, 189, 108, 234, 203, 218,  50,  85, 172,  63,  80, 206,  94,  17, 127, 225,  37,  74,  58,  20, 198, 249, 234,  73,  93,  57, 165,  24, 188, 223},
                                       {175,  38,  21, 159,  86,  43, 164, 132, 115, 143, 211,  32,  22, 219, 151,  76, 190, 102, 212, 253, 113, 166,  99, 147,  46,  13, 132, 255, 141,  79, 161,  40},
                                       {246, 115, 199,  53, 122, 243,  16,  77,   9, 229, 176, 105, 240, 131, 170, 202,  50, 139,  64, 152, 237,  39,   5, 184, 116, 177, 193, 208,   7,  49, 243, 111},
                                       { 81,  68, 225, 104, 188, 218,  97, 170, 191,  37,  91, 157,  53,   3,  68,  35, 118, 244,  10,  93, 204, 174,  55, 242,  86, 226,  36, 101, 231, 150,  28, 182},
                                       {  9, 126, 168,  30,   1, 139,  62, 149, 252, 121,  68, 196, 162, 111, 212,  85, 233, 163, 178,  47,  81, 127,  66, 140,  29, 153,  20, 125,  62, 200,  91, 132},
                                       {212, 186, 150, 254, 112, 179, 241,  24,  53, 222, 137,  27, 225, 254, 145, 184,  58, 130,  33, 194,  16, 224, 214, 107, 205, 248,  79, 172, 161, 252,  13,  71},
                                       {229,  41,  92, 233,  45,  66, 160,  87, 110, 237,  18,  42,  82, 124,  21,  98,  44, 248, 109, 216, 146,   0, 163,  11,  51,  96, 143,   5,  84,  44, 213, 222},
                                       {103,  57,  17, 135,  79, 208, 128,  34, 201, 157, 182, 148, 214,  70,  10, 159, 221,  76,   4,  95, 156, 123,  72, 177, 135, 185, 234, 216, 112, 192,  96, 122},
                                       {164, 179, 119, 204, 191,  10, 173, 219,   3,  75,  56, 104, 199, 115, 205,  88, 176, 139, 189, 235, 210,  61, 230,  86, 198,  14, 127,  65,  31, 138,  54, 153},
                                       {  1, 149, 223,  28,  52, 245, 118,  61, 146, 132, 251,  15, 244,  37, 172, 232,  18, 125,  30,  53, 113,  20,  36, 246,  24, 155,  43, 246, 166, 207, 178,  22},
                                       {197,  70, 251, 115, 102, 156,  19, 209,  46, 190,  30, 162,  91,  49, 135,  73, 186,  64, 241, 165, 136, 221, 193, 145, 116,  75, 105, 192,  93,   8,  72, 217},
                                       { 98, 129,  40,  84, 183, 140, 198,  95, 233, 114, 227, 125, 218, 154, 238, 106,   0, 146, 200,  12,  87, 101, 169,  57, 238, 224, 211,  34, 134, 242, 149, 104},
                                       {190,  12, 208, 169, 230,  71,  37,  83, 167,  12,  78,  65, 178,   4,  85, 212, 120, 227, 108,  48, 154, 252,  31,  77,   5, 139, 176,  51, 255, 117,  41, 167},
                                       {236,  59, 154,  24, 215,   4, 126, 255, 181, 138, 242, 201,  99,  20, 171,  55,  42,  92, 217, 183,  74, 204, 128, 186, 158,  97,  22,  82, 155,  66, 188,  56},
                                       { 45, 143,  89, 133, 244,  56, 100, 148,  26,  65, 106,  39, 147, 250, 234, 194, 160,  82, 173,  34, 117, 231,  18,  69, 226, 243, 109, 200, 216, 128,  26, 239},
                                       {103, 254, 195,  77, 111, 158, 174, 228,  51, 215, 157, 187,  75,  61, 113, 130,  23, 247, 138,   6,  60, 146, 105,  40, 197, 121,  60, 165,   6, 230,  89, 210},
                                       {168,   7, 224,  29,  43, 203,  16, 190, 118,   6,  90, 232,  44, 141, 180,  11,  67, 199, 123, 237, 164, 250, 213,  52, 177,  47,   8, 246, 141, 103,  30, 120},
                                       {203,  64, 120, 181,  94, 238, 136,  35,  78, 247, 167, 127,  32, 220,  89, 207, 151,  38,  98, 187,  84,  27, 131,  92, 220, 148,  76, 205,  72, 180,  15, 161}},
                                      {{137, 241,  39, 152, 214, 191,  88, 224,  22, 182,  87, 150,  27, 173,  71, 123, 184,  25,  65, 178,   8, 238,  34, 209, 100, 176,  66, 194, 213,  43, 117, 245},
                                       { 23,  75, 127, 103,  14,  65, 141, 118,  47, 244, 128, 212, 237, 103,  46, 239, 205, 223, 135,  92, 114, 153,  58, 121, 252, 145,  19,  91, 237, 108,  11,  96},
                                       {231, 180, 203,  56, 177,  34, 239, 206, 164,  75, 105,   8, 195, 159,  17, 139,  54,  88, 164, 253, 219, 195, 110, 166,  12,  49, 130, 161,  35, 175, 150, 220},
                                       {116, 159,   7, 220, 246, 102,  78,   5,  54, 173,  39, 115,  57, 225, 188,  76,   6, 149,  37,  47,  84,  18,  24,  75, 187, 225, 201,   5,  81, 189,  25,  63},
                                       { 42,  91, 107,  70, 197, 166, 134, 152, 192, 227, 255, 146,  67, 131, 249, 170, 120, 232, 199, 104, 129, 172, 211, 244,  40,  79, 118, 248, 140, 238, 200, 128},
                                       {193, 254,  29, 141,  48,  19, 248,  37, 110,  89,  29,  98, 178,  22,  41,  92, 247, 186,  58, 123, 179, 230,  62, 155, 137, 217, 172,  60,  21,  76, 112,  43},
                                       {132, 217, 168, 122, 227,  94,  62, 209, 124,  15, 216, 163, 203, 105,  51, 129,  70,  28, 212, 144,  33,   3, 102, 127,  28,  11,  99, 149,  52, 181, 158, 249},
                                       { 80,  17,  61, 204,  85, 215, 131, 170, 235,  49, 139,  77,   4, 220, 172, 194, 155,  11,  96,  79, 160,  53, 229, 202,  89, 253, 190, 209, 125, 222,  65,  86},
                                       {233, 187, 113,  44, 156, 183,  74,   9, 103, 154, 239, 186,  59, 250, 107,  15, 226, 141, 240, 206, 221, 115, 167, 182, 111,  44,  73,  93, 236,   7, 211, 167},
                                       { 35, 101, 145, 243,   5, 112,  27, 252, 197,  69,  34, 118, 132, 149,  32, 122, 185,  42, 109,  22,   0, 148,  41,  17, 197, 142, 164,  30, 131,  38,  81, 142},
                                       { 53, 224, 199,  24, 175, 231,  57, 145,  85, 245,  20, 160,  45, 199,  66,  73, 211, 164,  60, 136, 189,  70, 242,  86,  55, 244, 214, 102, 154, 193, 106,  14},
                                       {178, 162,  71,  88,  97, 136, 191, 120, 222, 169,  93, 207, 218,  87, 233, 244,  99,   3, 238,  91, 175, 130,   9, 155,  97, 179,   2,  23, 203,  69, 216, 251},
                                       {140, 122,   8, 207, 218,  13, 160,  40,   4, 107, 143,   9, 180,   0, 157,  51,  19, 147, 194,  49, 214, 230, 204,  25, 120, 222,  66, 117, 228, 174,  54,  26},
                                       { 33, 239,  99, 151,  61, 249,  83, 184,  52, 229,  74, 192, 127,  81, 136, 113, 179, 221,  69, 119,  33, 108,  82, 169,  39, 186, 140,  78,  42, 126, 188, 112},
                                       {211,  77,  47, 189, 128,  32, 201, 132, 152, 255,  24, 101, 170, 247,  30, 163,  91,  80, 251,  17, 185, 143,  61, 236, 195,  51, 250, 165, 147,   6,  93, 233},
                                       {117, 157, 171, 228, 109,  53, 237, 114,  90, 209,  36,  60, 215,  49, 225, 205, 236,  38, 134,  68, 159, 247,   2, 133,  74,  12, 210,  88, 242, 219,  52, 154},
                                       { 66,  18, 253,   5,  73, 222,  20, 175,  11, 166, 198, 121, 153,   3,  64, 106, 124,   9, 168, 200,  96,  31, 117, 193, 100, 157, 111,  32,  99, 177,  72, 202},
                                       {196, 137,  90, 178, 147, 162,  98,  68,  46, 139,  80, 234, 183, 132,  78, 191, 146,  57, 226, 210,  45, 217, 173, 153, 228,  40, 255,  62, 198,  16, 133,  36},
                                       {238,  38, 218,  80,  28, 197, 214, 245, 188, 241, 108,  18,  95,  35, 212, 201,  25, 174, 110,  73,  21, 129,  86,  59,  21, 124, 180, 143, 235, 118,  84, 249},
                                       {101,  56, 123, 251, 105,  63, 135,  87, 156,  30,  54, 196, 168, 253, 116, 100,  43,  82, 241, 149,   0, 204, 235,  10, 206,  79, 215,   1,  57, 221, 169, 147},
                                       {208,   7, 166, 144, 229,  12,  40, 119,   4, 254, 144, 223,  89,  10,  55, 179, 213, 137,   2, 183, 142, 161, 101, 243, 165, 135, 106,  94, 158,  46, 191,  22},
                                       {185, 156,  72,  47, 205, 169, 224, 184,  55, 176, 126,  70,  46, 207, 151,  19, 122, 231,  94,  36,  60,  50,  15, 113,  28,  64, 232, 172,  31,  92,  69, 110},
                                       { 32, 231, 114,  23,  95, 128, 150,  76, 235, 100,  20, 159, 246, 134,  79, 162,  29,  67, 190, 223, 125, 232, 196,  76, 155, 201,  16, 188, 250, 127, 243, 206},
                                       { 85, 130, 240, 195, 213,  68,  26, 111, 208, 138,  63,  33, 186, 109, 228, 200, 146, 185,  50, 243, 167,  85, 174, 219, 254,  44,  84, 145, 217,   8, 138,  45},
                                       {226,  41, 173,   4,  82, 248, 189, 162,  10, 241, 193, 219, 120,  41,  71,  97,   0, 113,  10, 151,  24, 104,  37,  67, 130,  98, 119,  37, 104, 181,  78, 163},
                                       {148, 105,  56, 158, 141, 102,  48, 124,  38,  81,  94, 174, 144,   2, 215, 236, 171, 240, 106,  72, 248, 190, 138,   1, 182, 208, 165, 229,  59, 198,  20, 232},
                                       { 13, 245, 190,  31, 218,  16, 199, 230, 153, 205,  50,  18, 207,  87, 157,  16,  82, 131, 198,  43, 210,  56,  89, 237, 150,  72,  26, 192, 135, 154,  48,  92},
                                       {203,  96, 125,  63, 115, 171,  90, 255,  68, 133, 116, 167, 251,  77, 183, 123,  27, 220, 177,  95, 158,  14, 116, 223, 194,  52,  12, 242, 115,  64, 212, 126},
                                       {138,  27, 252, 182, 234,  55, 146,  26,   3, 181, 225,  29,  58, 196, 136,  48, 233,  61,   1, 142, 121, 227, 171,  31, 107, 125,  95, 176,   1, 252,  30, 170},
                                       { 74, 165,   6,  83, 134, 224,  75, 109, 210,  59, 247, 126, 104,   7, 226, 148,  71, 187, 111,  36, 254, 200,  64,   6, 161, 250, 216, 148,  67, 202, 108, 227},
                                       {221, 112, 208, 152,  42,  13, 168, 192, 156,  97, 143,  44, 161, 242,  35,  93, 163, 246,  51, 213,  77, 103, 129,  45, 204, 140,  39, 119,  83, 160,  15,  53},
                                       {176,  50,  65, 187,  98, 246, 121,  34, 234,  14,  84, 177, 216,  62, 114, 202,  13, 133,  83, 184, 151,  23, 234, 180,  86,  58, 230,  21, 181, 240,  90, 139}}};

#endif  // DITHER_KALLEBACH_DATA_H