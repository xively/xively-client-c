/* Copyright (c) 2003-2016, LogMeIn, Inc. All rights reserved.
 *
 * This is part of the Xively C Client library,
 * it is licensed under the BSD 3-Clause license.
 */

#include <xi_bsp_rng.h>
#include <stm32f4xx_hal.h>

void xi_bsp_rng_init()
{ /* check if already initialized */
}

uint32_t xi_bsp_rng_get()
{
	/* fake random number for backoff logic with 1 / 1023 precision
	   the last ten bits change rapidly enough in the tick counter -> 1023
	   we need to project the 0..0x3FF interval to 0..0xFFFFFFFF interval, so the smallest number is 0xFFFFFFFF / 0x3FF
	   the multiplier should be also between 0..0x3FF so we AND ( 0x3FF + 1 ) with the tick counter
	   to add some fuzzyness we XOR with the random number 0x1B4 */

    uint32_t random = ( 0xFFFFFFFF / 0x3FF ) * ( ( HAL_GetTick() % ( 0x3FF + 1 ) ) ^ 0x1B4 );

    /* test run with 500 ms call frequency :

	.random : 116 487014864
	.random : 514 2157979656
	.random : 24 100761696
	.random : 534 2241947736
	.random : 44 184729776
	.random : 570 2393090280
	.random : 48 201523392
	.random : 718 3014454072
	.random : 196 822887184
	.random : 722 3031247688
	.random : 232 974029728
	.random : 742 3115215768`
	.random : 252 1057997808
	.random : 650 2728962600
	.random : 128 537395712
	.random : 670 2812930680
	.random : 148 621363792
	.random : 674 2829724296
	.random : 184 772506336
	.random : 694 2913692376
	.random : 332 1393870128
	.random : 858 3602230632
	.random : 336 1410663744
	.random : 878 3686198712
	.random : 356 1494631824
	.random : 882 3702992328
	.random : 264 1108378656
	.random : 774 3249564696
	.random : 284 1192346736
	.random : 810 3400707240
	.random : 288 1209140352
	.random : 830 3484675320
	.random : 308 1293108432
	.random : 962 4038864648
	.random : 472 1981646688
	.random : 982 4122832728
	.random : 492 2065614768
	.random : 1018 4273975272
	.random : 496 2082408384
	.random : 910 3820547640
	.random : 388 1628980752
	.random : 914 3837341256
	.random : 424 1780123296
	.random : 934 3921309336
	.random : 444 1864091376
	.random : 74 310681896
	.random : 576 2418280704
	.random : 94 394649976
	.random : 596 2502248784
	.random : 98 411443592
	.random : 632 2653391328
	.random : 118 495411672
	.random : 524 2199963696
	.random : 26 109158504
	.random : 528 2216757312
	.random : 46 193126584
	.random : 548 2300725392
	.random : 50 209920200
	.random : 712 2989263648
	.random : 198 831283992
	.random : 732 3073231728
	.random : 234 982426536
	.random : 736 3090025344
	.random : 254 1066394616
	.random : 756 3173993424
	.random : 130 545792520
	.random : 664 2787740256
	.random : 150 629760600
	.random : 684 2871708336
	.random : 186 780903144
	.random : 688 2888501952
	.random : 334 1402266936
	.random : 836 3509865744
	.random : 338 1419060552
	.random : 872 3661008288
	.random : 358 1503028632
	.random : 892 3744976368
	.random : 266 1116775464
	.random : 768 3224374272
	.random : 286 1200743544
	.random : 788 3308342352
	.random : 290 1217537160
	.random : 824 3459484896
	.random : 310 1301505240
	.random : 972 4080848688
	.random : 474 1990043496
	.random : 976 4097642304
	.random : 494 2074011576
	.random : 996 4181610384
	.random : 498 2090805192
	.random : 904 3795357216
	.random : 390 1637377560
	.random : 924 3879325296 */

    return random;
}

void xi_bsp_rng_shutdown()
{
}