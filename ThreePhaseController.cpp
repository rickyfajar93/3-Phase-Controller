/* 
 * File:   ThreePhaseController.cpp
 * Author: Cameron
 * 
 * Created on October 22, 2015, 2:21 AM
 */

#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "ThreePhaseController.h"
#include "MLX90363.h"
#include "ThreePhaseDriver.h"
#include "Debug.h"

u4 ThreePhaseController::drivePhase;
s2 ThreePhaseController::driveVelocity;
bool ThreePhaseController::isForward;

void TIMER4_OVF_vect() {
 ThreePhaseController::isr();
}

void ThreePhaseController::isr() {
 u1 static mlx = 1;
 
 auto ph = drivePhase;
 ph += driveVelocity;
 if (ph > ThreePhaseDriver::StepsPerCycle) {
  if (driveVelocity > 0)
   ph -= ThreePhaseDriver::StepsPerCycle;
  else
   ph += ThreePhaseDriver::StepsPerCycle;
 }
 drivePhase = ph;
 ThreePhaseDriver::advanceTo(ph >> drivePhaseValueShift);
 
 // Don't continue if we're not done counting down
 if (--mlx)
  return;
 
 MLX90363::startTransmitting();
 
 mlx = cyclesPWMPerMLX;
}

u2 constexpr loop = 4681;

/**
 * 14-bit lookup table for magnetometer Alpha value to Phase value
 */
static const u2 AlphaToPhaseLookup[loop] PROGMEM = {
 // Table generated with node.js:
 // for(var i=0,l='';i<4681;){l+=String('   '+(Math.round((offset+i)*3*7*256/16384)%768)).slice(-3)+',';if(++i%31==0){console.log(l);l=''}}console.log(l)
 537,537,537,536,536,536,535,535,535,534,534,534,533,533,533,532,532,532,531,531,531,530,530,530,529,529,529,528,528,528,527,
 527,527,526,526,526,525,525,525,524,524,524,523,523,523,522,522,522,521,521,521,520,520,520,519,519,519,518,518,518,518,517,
 517,517,516,516,516,515,515,515,514,514,514,513,513,513,512,512,512,511,511,511,510,510,510,509,509,509,508,508,508,507,507,
 507,506,506,506,505,505,505,504,504,504,503,503,503,502,502,502,501,501,501,500,500,500,499,499,499,498,498,498,497,497,497,
 497,496,496,496,495,495,495,494,494,494,493,493,493,492,492,492,491,491,491,490,490,490,489,489,489,488,488,488,487,487,487,
 486,486,486,485,485,485,484,484,484,483,483,483,482,482,482,481,481,481,480,480,480,479,479,479,478,478,478,477,477,477,476,
 476,476,476,475,475,475,474,474,474,473,473,473,472,472,472,471,471,471,470,470,470,469,469,469,468,468,468,467,467,467,466,
 466,466,465,465,465,464,464,464,463,463,463,462,462,462,461,461,461,460,460,460,459,459,459,458,458,458,457,457,457,456,456,
 456,455,455,455,455,454,454,454,453,453,453,452,452,452,451,451,451,450,450,450,449,449,449,448,448,448,447,447,447,446,446,
 446,445,445,445,444,444,444,443,443,443,442,442,442,441,441,441,440,440,440,439,439,439,438,438,438,437,437,437,436,436,436,
 435,435,435,434,434,434,434,433,433,433,432,432,432,431,431,431,430,430,430,429,429,429,428,428,428,427,427,427,426,426,426,
 425,425,425,424,424,424,423,423,423,422,422,422,421,421,421,420,420,420,419,419,419,418,418,418,417,417,417,416,416,416,415,
 415,415,414,414,414,413,413,413,413,412,412,412,411,411,411,410,410,410,409,409,409,408,408,408,407,407,407,406,406,406,405,
 405,405,404,404,404,403,403,403,402,402,402,401,401,401,400,400,400,399,399,399,398,398,398,397,397,397,396,396,396,395,395,
 395,394,394,394,393,393,393,392,392,392,392,391,391,391,390,390,390,389,389,389,388,388,388,387,387,387,386,386,386,385,385,
 385,384,384,384,383,383,383,382,382,382,381,381,381,380,380,380,379,379,379,378,378,378,377,377,377,376,376,376,375,375,375,
 374,374,374,373,373,373,372,372,372,371,371,371,371,370,370,370,369,369,369,368,368,368,367,367,367,366,366,366,365,365,365,
 364,364,364,363,363,363,362,362,362,361,361,361,360,360,360,359,359,359,358,358,358,357,357,357,356,356,356,355,355,355,354,
 354,354,353,353,353,352,352,352,351,351,351,350,350,350,350,349,349,349,348,348,348,347,347,347,346,346,346,345,345,345,344,
 344,344,343,343,343,342,342,342,341,341,341,340,340,340,339,339,339,338,338,338,337,337,337,336,336,336,335,335,335,334,334,
 334,333,333,333,332,332,332,331,331,331,330,330,330,329,329,329,329,328,328,328,327,327,327,326,326,326,325,325,325,324,324,
 324,323,323,323,322,322,322,321,321,321,320,320,320,319,319,319,318,318,318,317,317,317,316,316,316,315,315,315,314,314,314,
 313,313,313,312,312,312,311,311,311,310,310,310,309,309,309,308,308,308,308,307,307,307,306,306,306,305,305,305,304,304,304,
 303,303,303,302,302,302,301,301,301,300,300,300,299,299,299,298,298,298,297,297,297,296,296,296,295,295,295,294,294,294,293,
 293,293,292,292,292,291,291,291,290,290,290,289,289,289,288,288,288,287,287,287,287,286,286,286,285,285,285,284,284,284,283,
 283,283,282,282,282,281,281,281,280,280,280,279,279,279,278,278,278,277,277,277,276,276,276,275,275,275,274,274,274,273,273,
 273,272,272,272,271,271,271,270,270,270,269,269,269,268,268,268,267,267,267,266,266,266,266,265,265,265,264,264,264,263,263,
 263,262,262,262,261,261,261,260,260,260,259,259,259,258,258,258,257,257,257,256,256,256,255,255,255,254,254,254,253,253,253,
 252,252,252,251,251,251,250,250,250,249,249,249,248,248,248,247,247,247,246,246,246,245,245,245,245,244,244,244,243,243,243,
 242,242,242,241,241,241,240,240,240,239,239,239,238,238,238,237,237,237,236,236,236,235,235,235,234,234,234,233,233,233,232,
 232,232,231,231,231,230,230,230,229,229,229,228,228,228,227,227,227,226,226,226,225,225,225,224,224,224,224,223,223,223,222,
 222,222,221,221,221,220,220,220,219,219,219,218,218,218,217,217,217,216,216,216,215,215,215,214,214,214,213,213,213,212,212,
 212,211,211,211,210,210,210,209,209,209,208,208,208,207,207,207,206,206,206,205,205,205,204,204,204,203,203,203,203,202,202,
 202,201,201,201,200,200,200,199,199,199,198,198,198,197,197,197,196,196,196,195,195,195,194,194,194,193,193,193,192,192,192,
 191,191,191,190,190,190,189,189,189,188,188,188,187,187,187,186,186,186,185,185,185,184,184,184,183,183,183,182,182,182,182,
 181,181,181,180,180,180,179,179,179,178,178,178,177,177,177,176,176,176,175,175,175,174,174,174,173,173,173,172,172,172,171,
 171,171,170,170,170,169,169,169,168,168,168,167,167,167,166,166,166,165,165,165,164,164,164,163,163,163,162,162,162,161,161,
 161,161,160,160,160,159,159,159,158,158,158,157,157,157,156,156,156,155,155,155,154,154,154,153,153,153,152,152,152,151,151,
 151,150,150,150,149,149,149,148,148,148,147,147,147,146,146,146,145,145,145,144,144,144,143,143,143,142,142,142,141,141,141,
 140,140,140,140,139,139,139,138,138,138,137,137,137,136,136,136,135,135,135,134,134,134,133,133,133,132,132,132,131,131,131,
 130,130,130,129,129,129,128,128,128,127,127,127,126,126,126,125,125,125,124,124,124,123,123,123,122,122,122,121,121,121,120,
 120,120,119,119,119,119,118,118,118,117,117,117,116,116,116,115,115,115,114,114,114,113,113,113,112,112,112,111,111,111,110,
 110,110,109,109,109,108,108,108,107,107,107,106,106,106,105,105,105,104,104,104,103,103,103,102,102,102,101,101,101,100,100,
 100, 99, 99, 99, 98, 98, 98, 98, 97, 97, 97, 96, 96, 96, 95, 95, 95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 91, 90, 90,
  90, 89, 89, 89, 88, 88, 88, 87, 87, 87, 86, 86, 86, 85, 85, 85, 84, 84, 84, 83, 83, 83, 82, 82, 82, 81, 81, 81, 80, 80, 80,
  79, 79, 79, 78, 78, 78, 77, 77, 77, 77, 76, 76, 76, 75, 75, 75, 74, 74, 74, 73, 73, 73, 72, 72, 72, 71, 71, 71, 70, 70, 70,
  69, 69, 69, 68, 68, 68, 67, 67, 67, 66, 66, 66, 65, 65, 65, 64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 61, 60, 60, 60, 59,
  59, 59, 58, 58, 58, 57, 57, 57, 56, 56, 56, 56, 55, 55, 55, 54, 54, 54, 53, 53, 53, 52, 52, 52, 51, 51, 51, 50, 50, 50, 49,
  49, 49, 48, 48, 48, 47, 47, 47, 46, 46, 46, 45, 45, 45, 44, 44, 44, 43, 43, 43, 42, 42, 42, 41, 41, 41, 40, 40, 40, 39, 39,
  39, 38, 38, 38, 37, 37, 37, 36, 36, 36, 35, 35, 35, 35, 34, 34, 34, 33, 33, 33, 32, 32, 32, 31, 31, 31, 30, 30, 30, 29, 29,
  29, 28, 28, 28, 27, 27, 27, 26, 26, 26, 25, 25, 25, 24, 24, 24, 23, 23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 20, 19, 19, 19,
  18, 18, 18, 17, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10,  9,  9,  9,
   8,  8,  8,  7,  7,  7,  6,  6,  6,  5,  5,  5,  4,  4,  4,  3,  3,  3,  2,  2,  2,  1,  1,  1,  0,  0,  0,767,767,767,766,
 766,766,765,765,765,764,764,764,763,763,763,762,762,762,761,761,761,761,760,760,760,759,759,759,758,758,758,757,757,757,756,
 756,756,755,755,755,754,754,754,753,753,753,752,752,752,751,751,751,750,750,750,749,749,749,748,748,748,747,747,747,746,746,
 746,745,745,745,744,744,744,743,743,743,742,742,742,741,741,741,740,740,740,740,739,739,739,738,738,738,737,737,737,736,736,
 736,735,735,735,734,734,734,733,733,733,732,732,732,731,731,731,730,730,730,729,729,729,728,728,728,727,727,727,726,726,726,
 725,725,725,724,724,724,723,723,723,722,722,722,721,721,721,720,720,720,719,719,719,719,718,718,718,717,717,717,716,716,716,
 715,715,715,714,714,714,713,713,713,712,712,712,711,711,711,710,710,710,709,709,709,708,708,708,707,707,707,706,706,706,705,
 705,705,704,704,704,703,703,703,702,702,702,701,701,701,700,700,700,699,699,699,698,698,698,698,697,697,697,696,696,696,695,
 695,695,694,694,694,693,693,693,692,692,692,691,691,691,690,690,690,689,689,689,688,688,688,687,687,687,686,686,686,685,685,
 685,684,684,684,683,683,683,682,682,682,681,681,681,680,680,680,679,679,679,678,678,678,677,677,677,677,676,676,676,675,675,
 675,674,674,674,673,673,673,672,672,672,671,671,671,670,670,670,669,669,669,668,668,668,667,667,667,666,666,666,665,665,665,
 664,664,664,663,663,663,662,662,662,661,661,661,660,660,660,659,659,659,658,658,658,657,657,657,656,656,656,656,655,655,655,
 654,654,654,653,653,653,652,652,652,651,651,651,650,650,650,649,649,649,648,648,648,647,647,647,646,646,646,645,645,645,644,
 644,644,643,643,643,642,642,642,641,641,641,640,640,640,639,639,639,638,638,638,637,637,637,636,636,636,635,635,635,635,634,
 634,634,633,633,633,632,632,632,631,631,631,630,630,630,629,629,629,628,628,628,627,627,627,626,626,626,625,625,625,624,624,
 624,623,623,623,622,622,622,621,621,621,620,620,620,619,619,619,618,618,618,617,617,617,616,616,616,615,615,615,614,614,614,
 614,613,613,613,612,612,612,611,611,611,610,610,610,609,609,609,608,608,608,607,607,607,606,606,606,605,605,605,604,604,604,
 603,603,603,602,602,602,601,601,601,600,600,600,599,599,599,598,598,598,597,597,597,596,596,596,595,595,595,594,594,594,593,
 593,593,593,592,592,592,591,591,591,590,590,590,589,589,589,588,588,588,587,587,587,586,586,586,585,585,585,584,584,584,583,
 583,583,582,582,582,581,581,581,580,580,580,579,579,579,578,578,578,577,577,577,576,576,576,575,575,575,574,574,574,573,573,
 573,572,572,572,572,571,571,571,570,570,570,569,569,569,568,568,568,567,567,567,566,566,566,565,565,565,564,564,564,563,563,
 563,562,562,562,561,561,561,560,560,560,559,559,559,558,558,558,557,557,557,556,556,556,555,555,555,554,554,554,553,553,553,
 552,552,552,551,551,551,551,550,550,550,549,549,549,548,548,548,547,547,547,546,546,546,545,545,545,544,544,544,543,543,543,
 542,542,542,541,541,541,540,540,540,539,539,539,538,538,538,537,537,537,536,536,536,535,535,535,534,534,534,533,533,533,532,
 532,532,531,531,531,530,530,530,530,529,529,529,528,528,528,527,527,527,526,526,526,525,525,525,524,524,524,523,523,523,522,
 522,522,521,521,521,520,520,520,519,519,519,518,518,518,517,517,517,516,516,516,515,515,515,514,514,514,513,513,513,512,512,
 512,511,511,511,510,510,510,509,509,509,509,508,508,508,507,507,507,506,506,506,505,505,505,504,504,504,503,503,503,502,502,
 502,501,501,501,500,500,500,499,499,499,498,498,498,497,497,497,496,496,496,495,495,495,494,494,494,493,493,493,492,492,492,
 491,491,491,490,490,490,489,489,489,488,488,488,488,487,487,487,486,486,486,485,485,485,484,484,484,483,483,483,482,482,482,
 481,481,481,480,480,480,479,479,479,478,478,478,477,477,477,476,476,476,475,475,475,474,474,474,473,473,473,472,472,472,471,
 471,471,470,470,470,469,469,469,468,468,468,467,467,467,467,466,466,466,465,465,465,464,464,464,463,463,463,462,462,462,461,
 461,461,460,460,460,459,459,459,458,458,458,457,457,457,456,456,456,455,455,455,454,454,454,453,453,453,452,452,452,451,451,
 451,450,450,450,449,449,449,448,448,448,447,447,447,446,446,446,446,445,445,445,444,444,444,443,443,443,442,442,442,441,441,
 441,440,440,440,439,439,439,438,438,438,437,437,437,436,436,436,435,435,435,434,434,434,433,433,433,432,432,432,431,431,431,
 430,430,430,429,429,429,428,428,428,427,427,427,426,426,426,425,425,425,425,424,424,424,423,423,423,422,422,422,421,421,421,
 420,420,420,419,419,419,418,418,418,417,417,417,416,416,416,415,415,415,414,414,414,413,413,413,412,412,412,411,411,411,410,
 410,410,409,409,409,408,408,408,407,407,407,406,406,406,405,405,405,404,404,404,404,403,403,403,402,402,402,401,401,401,400,
 400,400,399,399,399,398,398,398,397,397,397,396,396,396,395,395,395,394,394,394,393,393,393,392,392,392,391,391,391,390,390,
 390,389,389,389,388,388,388,387,387,387,386,386,386,385,385,385,384,384,384,383,383,383,383,382,382,382,381,381,381,380,380,
 380,379,379,379,378,378,378,377,377,377,376,376,376,375,375,375,374,374,374,373,373,373,372,372,372,371,371,371,370,370,370,
 369,369,369,368,368,368,367,367,367,366,366,366,365,365,365,364,364,364,363,363,363,362,362,362,362,361,361,361,360,360,360,
 359,359,359,358,358,358,357,357,357,356,356,356,355,355,355,354,354,354,353,353,353,352,352,352,351,351,351,350,350,350,349,
 349,349,348,348,348,347,347,347,346,346,346,345,345,345,344,344,344,343,343,343,342,342,342,341,341,341,341,340,340,340,339,
 339,339,338,338,338,337,337,337,336,336,336,335,335,335,334,334,334,333,333,333,332,332,332,331,331,331,330,330,330,329,329,
 329,328,328,328,327,327,327,326,326,326,325,325,325,324,324,324,323,323,323,322,322,322,321,321,321,320,320,320,320,319,319,
 319,318,318,318,317,317,317,316,316,316,315,315,315,314,314,314,313,313,313,312,312,312,311,311,311,310,310,310,309,309,309,
 308,308,308,307,307,307,306,306,306,305,305,305,304,304,304,303,303,303,302,302,302,301,301,301,300,300,300,299,299,299,299,
 298,298,298,297,297,297,296,296,296,295,295,295,294,294,294,293,293,293,292,292,292,291,291,291,290,290,290,289,289,289,288,
 288,288,287,287,287,286,286,286,285,285,285,284,284,284,283,283,283,282,282,282,281,281,281,280,280,280,279,279,279,278,278,
 278,278,277,277,277,276,276,276,275,275,275,274,274,274,273,273,273,272,272,272,271,271,271,270,270,270,269,269,269,268,268,
 268,267,267,267,266,266,266,265,265,265,264,264,264,263,263,263,262,262,262,261,261,261,260,260,260,259,259,259,258,258,258,
 257,257,257,257,256,256,256,255,255,255,254,254,254,253,253,253,252,252,252,251,251,251,250,250,250,249,249,249,248,248,248,
 247,247,247,246,246,246,245,245,245,244,244,244,243,243,243,242,242,242,241,241,241,240,240,240,239,239,239,238,238,238,237,
 237,237,236,236,236,236,235,235,235,234,234,234,233,233,233,232,232,232,231,231,231,230,230,230,229,229,229,228,228,228,227,
 227,227,226,226,226,225,225,225,224,224,224,223,223,223,222,222,222,221,221,221,220,220,220,219,219,219,218,218,218,217,217,
 217,216,216,216,215,215,215,215,214,214,214,213,213,213,212,212,212,211,211,211,210,210,210,209,209,209,208,208,208,207,207,
 207,206,206,206,205,205,205,204,204,204,203,203,203,202,202,202,201,201,201,200,200,200,199,199,199,198,198,198,197,197,197,
 196,196,196,195,195,195,194,194,194,194,193,193,193,192,192,192,191,191,191,190,190,190,189,189,189,188,188,188,187,187,187,
 186,186,186,185,185,185,184,184,184,183,183,183,182,182,182,181,181,181,180,180,180,179,179,179,178,178,178,177,177,177,176,
 176,176,175,175,175,174,174,174,173,173,173,173,172,172,172,171,171,171,170,170,170,169,169,169,168,168,168,167,167,167,166,
 166,166,165,165,165,164,164,164,163,163,163,162,162,162,161,161,161,160,160,160,159,159,159,158,158,158,157,157,157,156,156,
 156,155,155,155,154,154,154,153,153,153,152,152,152,152,151,151,151,150,150,150,149,149,149,148,148,148,147,147,147,146,146,
 146,145,145,145,144,144,144,143,143,143,142,142,142,141,141,141,140,140,140,139,139,139,138,138,138,137,137,137,136,136,136,
 135,135,135,134,134,134,133,133,133,132,132,132,131,131,131,131,130,130,130,129,129,129,128,128,128,127,127,127,126,126,126,
 125,125,125,124,124,124,123,123,123,122,122,122,121,121,121,120,120,120,119,119,119,118,118,118,117,117,117,116,116,116,115,
 115,115,114,114,114,113,113,113,112,112,112,111,111,111,110,110,110,110,109,109,109,108,108,108,107,107,107,106,106,106,105,
 105,105,104,104,104,103,103,103,102,102,102,101,101,101,100,100,100, 99, 99, 99, 98, 98, 98, 97, 97, 97, 96, 96, 96, 95, 95,
  95, 94, 94, 94, 93, 93, 93, 92, 92, 92, 91, 91, 91, 90, 90, 90, 89, 89, 89, 89, 88, 88, 88, 87, 87, 87, 86, 86, 86, 85, 85,
  85, 84, 84, 84, 83, 83, 83, 82, 82, 82, 81, 81, 81, 80, 80, 80, 79, 79, 79, 78, 78, 78, 77, 77, 77, 76, 76, 76, 75, 75, 75,
  74, 74, 74, 73, 73, 73, 72, 72, 72, 71, 71, 71, 70, 70, 70, 69, 69, 69, 68, 68, 68, 68, 67, 67, 67, 66, 66, 66, 65, 65, 65,
  64, 64, 64, 63, 63, 63, 62, 62, 62, 61, 61, 61, 60, 60, 60, 59, 59, 59, 58, 58, 58, 57, 57, 57, 56, 56, 56, 55, 55, 55, 54,
  54, 54, 53, 53, 53, 52, 52, 52, 51, 51, 51, 50, 50, 50, 49, 49, 49, 48, 48, 48, 47, 47, 47, 47, 46, 46, 46, 45, 45, 45, 44,
  44, 44, 43, 43, 43, 42, 42, 42, 41, 41, 41, 40, 40, 40, 39, 39, 39, 38, 38, 38, 37, 37, 37, 36, 36, 36, 35, 35, 35, 34, 34,
  34, 33, 33, 33, 32, 32, 32, 31, 31, 31, 30, 30, 30, 29, 29, 29, 28, 28, 28, 27, 27, 27, 26, 26, 26, 26, 25, 25, 25, 24, 24,
  24, 23, 23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 17, 16, 16, 16, 15, 15, 15, 14, 14, 14,
  13, 13, 13, 12, 12, 12, 11, 11, 11, 10, 10, 10,  9,  9,  9,  8,  8,  8,  7,  7,  7,  6,  6,  6,  5,  5,  5,  5,  4,  4,  4,
   3,  3,  3,  2,  2,  2,  1,  1,  1,  0,  0,  0,767,767,767,766,766,766,765,765,765,764,764,764,763,763,763,762,762,762,761,
 761,761,760,760,760,759,759,759,758,758,758,757,757,757,756,756,756,755,755,755,754,754,754,753,753,753,752,752,752,752,751,
 751,751,750,750,750,749,749,749,748,748,748,747,747,747,746,746,746,745,745,745,744,744,744,743,743,743,742,742,742,741,741,
 741,740,740,740,739,739,739,738,738,738,737,737,737,736,736,736,735,735,735,734,734,734,733,733,733,732,732,732,731,731,731,
 731,730,730,730,729,729,729,728,728,728,727,727,727,726,726,726,725,725,725,724,724,724,723,723,723,722,722,722,721,721,721,
 720,720,720,719,719,719,718,718,718,717,717,717,716,716,716,715,715,715,714,714,714,713,713,713,712,712,712,711,711,711,710,
 710,710,710,709,709,709,708,708,708,707,707,707,706,706,706,705,705,705,704,704,704,703,703,703,702,702,702,701,701,701,700,
 700,700,699,699,699,698,698,698,697,697,697,696,696,696,695,695,695,694,694,694,693,693,693,692,692,692,691,691,691,690,690,
 690,689,689,689,689,688,688,688,687,687,687,686,686,686,685,685,685,684,684,684,683,683,683,682,682,682,681,681,681,680,680,
 680,679,679,679,678,678,678,677,677,677,676,676,676,675,675,675,674,674,674,673,673,673,672,672,672,671,671,671,670,670,670,
 669,669,669,668,668,668,668,667,667,667,666,666,666,665,665,665,664,664,664,663,663,663,662,662,662,661,661,661,660,660,660,
 659,659,659,658,658,658,657,657,657,656,656,656,655,655,655,654,654,654,653,653,653,652,652,652,651,651,651,650,650,650,649,
 649,649,648,648,648,647,647,647,647,646,646,646,645,645,645,644,644,644,643,643,643,642,642,642,641,641,641,640,640,640,639,
 639,639,638,638,638,637,637,637,636,636,636,635,635,635,634,634,634,633,633,633,632,632,632,631,631,631,630,630,630,629,629,
 629,628,628,628,627,627,627,626,626,626,626,625,625,625,624,624,624,623,623,623,622,622,622,621,621,621,620,620,620,619,619,
 619,618,618,618,617,617,617,616,616,616,615,615,615,614,614,614,613,613,613,612,612,612,611,611,611,610,610,610,609,609,609,
 608,608,608,607,607,607,606,606,606,605,605,605,605,604,604,604,603,603,603,602,602,602,601,601,601,600,600,600,599,599,599,
 598,598,598,597,597,597,596,596,596,595,595,595,594,594,594,593,593,593,592,592,592,591,591,591,590,590,590,589,589,589,588,
 588,588,587,587,587,586,586,586,585,585,585,584,584,584,584,583,583,583,582,582,582,581,581,581,580,580,580,579,579,579,578,
 578,578,577,577,577,576,576,576,575,575,575,574,574,574,573,573,573,572,572,572,571,571,571,570,570,570,569,569,569,568,568,
 568,567,567,567,566,566,566,565,565,565,564,564,564,563,563,563,563,562,562,562,561,561,561,560,560,560,559,559,559,558,558,
 558,557,557,557,556,556,556,555,555,555,554,554,554,553,553,553,552,552,552,551,551,551,550,550,550,549,549,549,548,548,548,
 547,547,547,546,546,546,545,545,545,544,544,544,543,543,543,542,542,542,542,541,541,541,540,540,540,539,539,539,538,538,538,
};

/**
 * Converts the magnetometer to a phase value
 * @param alpha 14-bit value from magnetometer
 * @return phase value (0 - 0x2ff inclusive)
 */
inline static u2 lookupAlphaToPhase(u2 alpha) {
 // Make sure we're working with a 14-bit number
 alpha &= 0x7fff;
 
 // We could use a loop or a modulo operation but since we know the range of our
 // inputs, 14-bits, we can see that loop*7 covers all possible values of our
 // inputs. Three compares and optional subtractions are easy operations.
 
 if (alpha >= loop * 4) alpha -= loop * 4;
 if (alpha >= loop * 2) alpha -= loop * 2;
 if (alpha >= loop * 1) alpha -= loop * 1;
 
 // Read the phase number word from the calculated place in the lookup table
 return pgm_read_word(&AlphaToPhaseLookup[alpha]);
}

void ThreePhaseController::init() {
 MLX90363::init();
 ThreePhaseDriver::init();
 driveVelocity = 0;
 ThreePhaseDriver::setAmplitude(0);
 
 MLX90363::prepareGET1Message(MLX90363::MessageType::Alpha);
 
 // Initialize phase
 MLX90363::startTransmitting();
 while (!MLX90363::isMeasurementReady());
 MLX90363::startTransmitting();
 while (MLX90363::isTransmitting());
 drivePhase = lookupAlphaToPhase(MLX90363::getAlpha()) << drivePhaseValueShift;

 TIMSK4 = 1 << TOIE4;
}

void ThreePhaseController::setTorque(const Torque t) {
 isForward = t.forward;
 ThreePhaseDriver::setAmplitude(t.amplitude);
}

void ThreePhaseController::updateDriver() {
 
 // Make sure we only run this if we have new data
 static u1 roll = 0xff;
 const u1 r = MLX90363::getRoll();
 if (r == roll) return;
 roll = r;
 
 static u2 lastPosition = drivePhase >> drivePhaseValueShift;
 
 // We can always grab the latest Alpha value safely here
 u2 const alpha = MLX90363::getAlpha();
 u2 pos = lookupAlphaToPhase(alpha);
 
 // Calculate the velocity from the magnetic data
 s2 velocity = pos - lastPosition;
 
 // Save the most recent magnetic position
 lastPosition = pos;
 
 // Adjust the driveVelocity to match what the magnetometer things it is
 if (velocity > driveVelocity) {
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
   driveVelocity++;
  }
 } else if (velocity < driveVelocity) {
  ATOMIC_BLOCK(ATOMIC_FORCEON) {
   driveVelocity--;
  }
 }
}
