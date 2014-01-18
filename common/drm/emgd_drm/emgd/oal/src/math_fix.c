/*
 *-----------------------------------------------------------------------------
 * Filename: math_fix.c
 * $Revision: 1.6 $
 *-----------------------------------------------------------------------------
 * Copyright (c) 2002-2010, Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *-----------------------------------------------------------------------------
 * Description:
 *  This file contains fixed point implementations of math functions
 *-----------------------------------------------------------------------------
 */


/******************************************************************************
 * This is a look-up table for natural log for integer values 1 - 255 in
 * 24i.8f fixed point format.
 *
 * This table returns 0 or ln(0), but it is technically undefined.
 *****************************************************************************/
static const int ln_table[] =
  {
    0,    0,  177,  281,  355,  412,  459,  498,  532,  562,  589,  614,
    636,  657,  676,  693,  710,  725,  740,  754,  767,  779,  791,  803,
    814,  824,  834,  844,  853,  862,  871,  879,  887,  895,  903,  910,
    917,  924,  931,  938,  944,  951,  957,  963,  969,  975,  980,  986,
    991,  996, 1001, 1007, 1012, 1016, 1021, 1026, 1030, 1035, 1039, 1044,
    1048, 1052, 1057, 1061, 1065, 1069, 1073, 1076, 1080, 1084, 1088, 1091,
    1095, 1098, 1102, 1105, 1109, 1112, 1115, 1119, 1122, 1125, 1128, 1131,
    1134, 1137, 1140, 1143, 1146, 1149, 1152, 1155, 1158, 1160, 1163, 1166,
    1168, 1171, 1174, 1176, 1179, 1181, 1184, 1186, 1189, 1191, 1194, 1196,
    1199, 1201, 1203, 1206, 1208, 1210, 1212, 1215, 1217, 1219, 1221, 1223,
    1226, 1228, 1230, 1232, 1234, 1236, 1238, 1240, 1242, 1244, 1246, 1248,
    1250, 1252, 1254, 1256, 1258, 1260, 1261, 1263, 1265, 1267, 1269, 1270,
    1272, 1274, 1276, 1278, 1279, 1281, 1283, 1284, 1286, 1288, 1289, 1291,
    1293, 1294, 1296, 1298, 1299, 1301, 1302, 1304, 1306, 1307, 1309, 1310,
    1312, 1313, 1315, 1316, 1318, 1319, 1321, 1322, 1324, 1325, 1327, 1328,
    1329, 1331, 1332, 1334, 1335, 1336, 1338, 1339, 1341, 1342, 1343, 1345,
    1346, 1347, 1349, 1350, 1351, 1353, 1354, 1355, 1356, 1358, 1359, 1360,
    1361, 1363, 1364, 1365, 1366, 1368, 1369, 1370, 1371, 1372, 1374, 1375,
    1376, 1377, 1378, 1380, 1381, 1382, 1383, 1384, 1385, 1387, 1388, 1389,
    1390, 1391, 1392, 1393, 1394, 1395, 1397, 1398, 1399, 1400, 1401, 1402,
    1403, 1404, 1405, 1406, 1407, 1408, 1409, 1410, 1411, 1412, 1413, 1415,
    1416, 1417, 1418, 1419
  };



/*----------------------------------------------------------------------------
 *
 * Function: os_pow_fix()
 *
 * Parameters:
 *    [IN] base:  the base, should be between 0 and 255.
 *    [IN] power:  this must be in 24i.8f format.
 *
 * Description:
 *    This function uses the Taylor Series to approximate the power function
 *    using fixed-point math.  See this website for the math background
 *    http://www.efunda.com/math/taylor_series/exponential.cfm
 *
 *    This function was originally designed for gamma correction.  When the
 *    base value is between 1 - 255 and the power is between 0.1 and 2, this
 *    function will produce an approximation that is within 2% of the "real"
 *    function.
 *
 * Returns:
 *    The result in 24i.8f format
 *
 *----------------------------------------------------------------------------
 */

unsigned int os_pow_fix( const int base, const int power )
{
  /* For some reason using "const unsigned int" causes a compiler error
   * when I use it below when I declare "result[APPROXIMATE_TERMS]".  So
   * switched these from "const unsigned .." to #define
   */
  #define APPROXIMATE_TERMS     40
#define FRACTION_BITS         8    /* num of bits for fraction */

  unsigned int       result[APPROXIMATE_TERMS];
  unsigned int       i, power_loop;
  unsigned int       total  = 0;
  int                ln_x   = ln_table[base];  /* ln_x is in 24i.8f format */


  /* nothing to do if we get 0 */
  if (0 == base) {
    return 0;
  } else {
    ln_x = ln_table[base];
  }

  /* We approximate the result using APPROXIMATE_TERMS terms */
  for (i = 0; i < APPROXIMATE_TERMS; i++)    {

    result[i] = 1 << FRACTION_BITS;

    /* Need to be very careful about the order in which we are multiplying
     * multiplying and dividing because we don't want any overflow.  In
     * addition, every time we multiply 2 fixed point numbers, we need to
     * shift the result by FRACTION_BITS to maintain the radix point.
     */
    for( power_loop = 0; power_loop < i; power_loop++ )    {
      result[i] *= ln_x;
      result[i] /= (power_loop + 1);
      result[i] >>=FRACTION_BITS;
      result[i] *= power;
      result[i] >>=FRACTION_BITS;

    }

    total += result[i];

  }

  return total;
}

