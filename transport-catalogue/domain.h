#pragma once

#include <string>
#include <vector>

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */
namespace domain {
	struct Stop {
		Stop() {}
		~Stop() {}
		std::string name;
		double lat;
		double lng;
	};

	const Stop EMPTY_STOP;

	struct Bus {
		Bus() {}
		~Bus() {}
		std::string name;
		std::vector<Stop*> bus;
		double lenght;
		double curvature;
		bool is_round;
	};

	const Bus EMPTY_BUS;
	const std::vector<Bus*> EMPTY_VEC_BUS;
}

