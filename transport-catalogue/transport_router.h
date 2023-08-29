#pragma once

#include "router.h"
#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <optional>
#include <iostream>
#include <memory>

using namespace std::literals;
using namespace graph;
template <typename Weight>
class TransportRouter {

public:

    TransportRouter() {}

	TransportRouter(TransportCatalogue& tc)
		: tc_(tc)
	{
		graph_ = std::make_unique<DirectedWeightedGraph<Weight>>(tc_.GetAllStopsWithBus().size() * 2);
	}

	void MakeGraph() {
		std::vector<Stop*> stops = tc_.GetAllStopsWithBus();
		size_t num_of_vertex = stops.size() * 2;
		double bus_wait_time = tc_.GetBusWaitTime();
		for (size_t s = 0; s * 2 < num_of_vertex; ++s) {
			Edge<Weight> a{ s * 2, (s * 2) + 1, bus_wait_time, stops[s]->name, 0};
			auto id = graph_->AddEdge(a);
			edge_id_.push_back(id);
			auto edge = graph_->GetEdge(id);
			added_edges_.push_back(edge);
		}
		std::vector<Bus*> buses = tc_.GetAllBuses();
		for (const auto Bus : buses) {
			if (Bus->is_round) {
				auto it = Bus->bus.begin();
				while (std::next(it) != Bus->bus.end()) {
					Edge<Weight> a;
					auto next_it = std::next(it);
					a.name = Bus->name;
					auto from = std::find_if(added_edges_.begin(), added_edges_.end(),
						[it](Edge<Weight> edge) {
							return (*it)->name == edge.name;
						});
					a.from = from->to;
					double distance = 0;
					while (next_it != Bus->bus.end() && (*it)->name != (*next_it)->name) {
						
						auto to = std::find_if(added_edges_.begin(), added_edges_.end(),
							[next_it](Edge<Weight> second_edge) {
								return (*next_it)->name == second_edge.name;
							});
						if (to != added_edges_.end()) {
							a.to = to->from;
							a.span_count = std::abs(std::distance(it, next_it));
							if (tc_.GetDistance(*std::prev(next_it), *next_it) != 0) {
								distance += tc_.GetDistance(*std::prev(next_it), *next_it);
							}
							else {
								distance += tc_.GetDistance(*next_it, *std::prev(next_it));
							}
							a.weight = distance / (tc_.GetBusVelocity() * 1000 / 60);
							auto id = graph_->AddEdge(a);
							edge_id_.push_back(id);
						}
						++next_it;
					}
					++it;
				}		
			}
			else {
				auto it = Bus->bus.begin();
				while (std::next(it) != Bus->bus.end()) {
					Edge<Weight> a;
					Edge<Weight> b;
					auto next_it = std::next(it);
					a.name = Bus->name;
					b.name = Bus->name;
					auto from = std::find_if(added_edges_.begin(), added_edges_.end(),
						[it](Edge<Weight> edge) {
							return (*it)->name == edge.name;
						});
					a.from = from->to;
					b.to = from->from;
					double distance = 0;
					double distance_backward = 0;
					while (next_it != Bus->bus.end()) {
						auto to = std::find_if(added_edges_.begin(), added_edges_.end(),
							[next_it](Edge<Weight> second_edge) {
								return (*next_it)->name == second_edge.name;
							});
						if (to != added_edges_.end()) {
							a.to = to->from;
							b.from = to->to;
							a.span_count = std::distance(it, next_it);
							b.span_count = a.span_count;
							//distance += tc_.GetDistance(*std::prev(next_it), *next_it);
							if (tc_.GetDistance(*std::prev(next_it), *next_it) != 0) {
								distance += tc_.GetDistance(*std::prev(next_it), *next_it);
							}
							else {
								distance += tc_.GetDistance(*next_it, *std::prev(next_it));
							}
							if (tc_.GetDistance(*next_it, *std::prev(next_it)) != 0) {
								distance_backward += tc_.GetDistance(*next_it, *std::prev(next_it));
							}
							else {
								distance_backward += tc_.GetDistance(*std::prev(next_it), *next_it);
							}
							a.weight = distance / (tc_.GetBusVelocity() * 1000 / 60);
							b.weight = distance_backward / (tc_.GetBusVelocity() * 1000 / 60);
							auto id_a = (*graph_).AddEdge(a);
							auto id_b = (*graph_).AddEdge(b);
							edge_id_.push_back(id_a);
							edge_id_.push_back(id_b);
							
						}
						++next_it;
					}
					++it;
				}
				
				// a
				/*
				auto it = Bus->bus.begin();
				while (std::next(it) != Bus->bus.end()) {
					Edge<Weight> a;
					auto next_it = std::next(it);
					a.name = Bus->name;
					auto from = std::find_if(added_edges_.begin(), added_edges_.end(),
						[it](Edge<Weight> edge) {
							return (*it)->name == edge.name;
						});
					a.from = from->to;
					double distance = 0;
					while (next_it != Bus->bus.end()) {
						auto to = std::find_if(added_edges_.begin(), added_edges_.end(),
							[next_it](Edge<Weight> second_edge) {
								return (*next_it)->name == second_edge.name;
							});
						if (to != added_edges_.end()) {
							a.to = to->from;
							a.span_count = std::abs(std::distance(it, next_it));
							if (tc_.GetDistance(*std::prev(next_it), *next_it) != 0) {
								distance += tc_.GetDistance(*std::prev(next_it), *next_it);
							}
							else {
								distance += tc_.GetDistance(*next_it, *std::prev(next_it));
							}
							//distance += tc_.GetDistance(*std::prev(next_it), *next_it);
							a.weight = distance / (tc_.GetBusVelocity() * 1000 / 60);
							auto id_a = graph_->AddEdge(a);
							edge_id_.push_back(id_a);
						}
						++next_it;
					}
					++it;
				}
				while (it != Bus->bus.begin()) {
					Edge<Weight> a;
					auto prev_it = std::prev(it);
					a.name = Bus->name;
					auto from = std::find_if(added_edges_.begin(), added_edges_.end(),
						[it](Edge<Weight> edge) {
							return (*it)->name == edge.name;
						});
					a.from = from->to;
					double distance = 0;
					while (prev_it != Bus->bus.begin()) {
						auto to = std::find_if(added_edges_.begin(), added_edges_.end(),
							[prev_it](Edge<Weight> second_edge) {
								return (*prev_it)->name == second_edge.name;
							});
						if (to != added_edges_.end()) {
							a.to = to->from;
							a.span_count = std::abs(std::distance(it, prev_it));
							//distance += tc_.GetDistance(*std::next(prev_it), *prev_it);
							if (tc_.GetDistance(*std::next(prev_it), *prev_it) != 0) {
								distance += tc_.GetDistance(*std::next(prev_it), *prev_it);
							}
							else {
								distance += tc_.GetDistance(*prev_it, *std::next(prev_it));
							}
							a.weight = distance / (tc_.GetBusVelocity() * 1000 / 60);
							auto id_a = graph_->AddEdge(a);
							edge_id_.push_back(id_a);
						}
						--prev_it;
					}
					--it;
				}
				*/
				// a

			}
		}
		//router_.push_back(Router<Weight>{*graph_});
		router_ = std::make_unique<Router<Weight>>(Router<Weight>{*graph_});

	}

	DirectedWeightedGraph<Weight>& GetGraph() {
		return graph_;
	}

	void BuildRoute(const Stop& left, const Stop& right, int request_id, std::ostream& cout) {
		auto from = std::find_if(added_edges_.begin(), added_edges_.end(),
			[left](Edge<Weight> edge) {
				return left.name == edge.name;
			});
		auto to = std::find_if(added_edges_.begin(), added_edges_.end(),
			[right](Edge<Weight> edge) {
				return right.name == edge.name;
			});
		if (from != added_edges_.end() && to != added_edges_.end()) {
			
			//auto info = router_[0].BuildRoute((*from).from, (*to).from);
			auto info = router_->BuildRoute((*from).from, (*to).from);
			if (info != std::nullopt) {
			
				json::Node a = json::Builder{}
					.StartDict()
					.Key("items"s).StartArray().EndArray()
					.Key("request_id"s).Value(request_id)
					.Key("total_time"s).Value((*info).weight)
					.EndDict()
					.Build();
				for (const auto& id : (*info).edges) {
					auto& edge = (*graph_).GetEdge(id);
					std::string name = { edge.name.begin(), edge.name.end() };
					if (edge.span_count == 0) {
						json::Node b = json::Builder{}
							.StartDict()
							.Key("stop_name"s).Value(name)
							.Key("time"s).Value(edge.weight)
							.Key("type"s).Value("Wait"s)
							.EndDict().Build();
						a.AsDict().at("items"s).AsArray().push_back(b.AsDict());
					}
					else {
						json::Node b = json::Builder{}
							.StartDict()
							.Key("bus"s).Value(name)
							.Key("span_count"s).Value(edge.span_count)
							.Key("time"s).Value(edge.weight)
							.Key("type"s).Value("Bus"s)
							.EndDict().Build();
						a.AsDict().at("items"s).AsArray().push_back(b.AsDict());
					}
				}
				json::Print(json::Document{ a }, cout);
			}
			else {
				json::Print(json::Document{ 
					json::Builder{}
					.StartDict()
					.Key("error_message"s).Value("not found"s)
					.Key("request_id"s).Value(request_id)
					.EndDict().Build() 
					}, cout);
			}
		}
		else {
			json::Print(json::Document{
				json::Builder{}
				.StartDict()
				.Key("error_message"s).Value("not found"s)
				.Key("request_id"s).Value(request_id)
				.EndDict().Build()
				}, cout);
		}
	}

private:
	TransportCatalogue& tc_;
	std::unique_ptr<DirectedWeightedGraph<Weight>> graph_;
	std::vector<Edge<Weight>> added_edges_;
	std::vector<EdgeId> edge_id_;
	//std::vector<Router<Weight>> router_;
	std::unique_ptr<Router<Weight>> router_;
};
