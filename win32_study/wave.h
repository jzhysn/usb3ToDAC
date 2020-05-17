#pragma once
#define SCALE_Y 20
#define SCALE_X 50
#define POINT_1 "*"
/* Standard error macro for reporting API errors */
#define PERR(bSuccess, api){if(!(bSuccess)) printf("%s:Error %d from %s \
on line %d\n", __FILE__, GetLastError(), api, __LINE__);}