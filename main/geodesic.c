#include "geodesic.h"
#include <math.h>

// Approximately 
#define RADIUS_OF_THE_EARTH 6371000 // meter 

double calculate_geodesic_distances_gnss(gnss_info_t *location1, gnss_info_t *location2) {
	if (location1 == NULL || location1 == NULL) return 0;

	double lat1 = decimal_to_radian(location1->latitude);
	double lon1 = decimal_to_radian(location1->longitude);
	double lat2 = decimal_to_radian(location2->latitude);
	double lon2 = decimal_to_radian(location2->longitude);

	return calculate_geodesic_distances_with_haversine(lat1, lon1, lat2, lon2);
}

double calculate_geodesic_distances(double lat1, double lon1, double lat2, double lon2) {
	return RADIUS_OF_THE_EARTH * acos(sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon1-lon2));
}

double calculate_geodesic_distances_with_haversine(double lat1, double lon1, double lat2, double lon2) {
	double dlat = lat2 - lat1;
	double dlon = lon2 - lon1;

	double a =
		sin(dlat / 2) * sin(dlat / 2) +
		cos(lat1) * cos(lat2) *
		sin(dlon / 2) * sin(dlon / 2);

	return RADIUS_OF_THE_EARTH * 2 * asin(sqrt(a));
} 

double decimal_to_radian(double decimal) {
	return (decimal * M_PI) / 180;
}


// Helper to convert NMEA (ddmm.mmmm or dddmm.mmmm) to decimal degrees
//static double nmea_to_decimal(double nmea_val, char hemisphere) {
	//int degrees = (int)(nmea_val / 100);
	//double minutes = nmea_val - (degrees * 100);
	//double decimal = degrees + (minutes / 60.0);
//
	//if (hemisphere == 'S' || hemisphere == 'W') {
		//decimal = -decimal;
	//}
	//return decimal;
//}
