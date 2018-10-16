/*!
 * \file geojson_printer.cc
 * \brief Implementation of a class that prints PVT solutions in GeoJSON format
 * \author Carles Fernandez-Prades, 2015. cfernandez(at)cttc.es
 *
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <https://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */


#include "geojson_printer.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <glog/logging.h>
#include <iomanip>
#include <sstream>


GeoJSON_Printer::GeoJSON_Printer()
{
    first_pos = true;
}


GeoJSON_Printer::~GeoJSON_Printer()
{
    GeoJSON_Printer::close_file();
}


bool GeoJSON_Printer::set_headers(std::string filename, bool time_tag_name)
{
    boost::posix_time::ptime pt = boost::posix_time::second_clock::local_time();
    tm timeinfo = boost::posix_time::to_tm(pt);

    if (time_tag_name)
        {
            std::stringstream strm0;
            const int year = timeinfo.tm_year - 100;
            strm0 << year;
            const int month = timeinfo.tm_mon + 1;
            if (month < 10)
                {
                    strm0 << "0";
                }
            strm0 << month;
            const int day = timeinfo.tm_mday;
            if (day < 10)
                {
                    strm0 << "0";
                }
            strm0 << day << "_";
            const int hour = timeinfo.tm_hour;
            if (hour < 10)
                {
                    strm0 << "0";
                }
            strm0 << hour;
            const int min = timeinfo.tm_min;
            if (min < 10)
                {
                    strm0 << "0";
                }
            strm0 << min;
            const int sec = timeinfo.tm_sec;
            if (sec < 10)
                {
                    strm0 << "0";
                }
            strm0 << sec;

            filename_ = filename + "_" + strm0.str() + ".geojson";
        }
    else
        {
            filename_ = filename + ".geojson";
        }

    geojson_file.open(filename_.c_str());

    first_pos = true;
    if (geojson_file.is_open())
        {
            DLOG(INFO) << "GeoJSON printer writing on " << filename.c_str();

            // Set iostream numeric format and precision
            geojson_file.setf(geojson_file.fixed, geojson_file.floatfield);
            geojson_file << std::setprecision(14);

            // Writing the header
            geojson_file << "{" << std::endl;
            geojson_file << "  \"type\":  \"Feature\"," << std::endl;
            geojson_file << "  \"properties\": {" << std::endl;
            geojson_file << "       \"name\": \"Locations generated by GNSS-SDR\" " << std::endl;
            geojson_file << "   }," << std::endl;


            geojson_file << "  \"geometry\": {" << std::endl;
            geojson_file << "      \"type\": \"MultiPoint\"," << std::endl;
            geojson_file << "      \"coordinates\": [" << std::endl;

            return true;
        }
    else
        {
            return false;
        }
}


bool GeoJSON_Printer::print_position(const std::shared_ptr<Pvt_Solution>& position, bool print_average_values)
{
    double latitude;
    double longitude;
    double height;

    std::shared_ptr<Pvt_Solution> position_ = position;

    if (print_average_values == false)
        {
            latitude = position_->get_latitude();
            longitude = position_->get_longitude();
            height = position_->get_height();
        }
    else
        {
            latitude = position_->get_avg_latitude();
            longitude = position_->get_avg_longitude();
            height = position_->get_avg_height();
        }

    if (geojson_file.is_open())
        {
            if (first_pos == true)
                {
                    geojson_file << "       [" << longitude << ", " << latitude << ", " << height << "]";
                    first_pos = false;
                }
            else
                {
                    geojson_file << "," << std::endl;
                    geojson_file << "       [" << longitude << ", " << latitude << ", " << height << "]";
                }
            return true;
        }
    else
        {
            return false;
        }
}


bool GeoJSON_Printer::close_file()
{
    if (geojson_file.is_open())
        {
            geojson_file << std::endl;
            geojson_file << "       ]" << std::endl;
            geojson_file << "   }" << std::endl;
            geojson_file << "}" << std::endl;
            geojson_file.close();

            // if nothing is written, erase the file
            if (first_pos == true)
                {
                    if (remove(filename_.c_str()) != 0) LOG(INFO) << "Error deleting temporary file";
                }

            return true;
        }
    else
        {
            return false;
        }
}