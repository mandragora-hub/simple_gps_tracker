#ifndef GEODESIC_H
#define GEODESIC_H

//https://www.studysmarter.es/resumenes/ingenieria/ingenieria-tecnologia-minera/distancia-geodesica/
//https://es.wikipedia.org/wiki/Ley_esf%C3%A9rica_de_los_cosenos
//https://en.wikipedia.org/wiki/Haversine_formula

#include "at/gnss.h"

double calculate_geodesic_distances_gnss(gnss_info_t *location1, gnss_info_t *location2);

double calculate_geodesic_distances(double lat1, double lon1, double lat2, double lon2);
double calculate_geodesic_distances_with_haversine(double lat1, double lon1, double lat2, double lon2);

double decimal_to_radian(double decimal);

#endif // GEODESIC_H
