#pragma once

#include "utils.h"

#include <SFML/Graphics/VertexArray.hpp>

#include <array>
#include <vector>

int GetVertex(WindXy pos) {
    int x = std::round(pos.x);
    int y = std::round(pos.y);
    return x / DPIXELS + (y / DPIXELS) * WIDTH / DPIXELS;
}

WindXy GetPos(int vertex) {
    int x = vertex % (WIDTH / DPIXELS);
    int y = vertex / (WIDTH / DPIXELS);
    return WindXy(x * DPIXELS, y * DPIXELS);
}

class Graph {
public:
    static inline const std::array<sf::Vector2f, 8> DIRS = {
        {
            {-1, -1},
            {0, -1},
            {1, -1},
            {1, 0},
            {1, 1},
            {0, 1},
            {-1, 1},
            {-1, 0},
        }
    };

    struct Edge {
        Edge(int from, int dirInd) : from(from), dirInd(dirInd) {
        }

        int from;
        int dirInd;
        int cost = 0;
    };

    Graph(int numVertices) : adjList_(numVertices) {
        for (auto& array : adjList_) {
            array.fill(std::numeric_limits<std::size_t>::max());
        }
        edges_.reserve(numVertices * DIRS.size());
    }

    Edge* AddEdge(int from, int dirInd) {
        adjList_[from][dirInd] = edges_.size();
        return &edges_.emplace_back(from, dirInd);
    }

    Edge* GetEdge(int from, int dirInd) {
        if (adjList_[from][dirInd] == -1) {
            return nullptr;
        }
        return &edges_[adjList_[from][dirInd]];
    }

    bool HasEdge(int from, int dirInd) const {
        return adjList_[from][dirInd] != -1;
    }

    void Render(sf::RenderWindow& window, float part = 0) const {
        for (auto edge : edges_) {
            sf::VertexArray line(sf::Lines);
            auto color = sf::Color(255, 0, 0, 255);
            color.r = std::max(static_cast<int>(color.r) + 10 * edge.cost, 0);
            color.b = std::min(static_cast<int>(color.b) - 10 * edge.cost, 255);
            line.append(sf::Vertex(GetPos(edge.from), color));
            line.append(sf::Vertex(GetPos(edge.from) + static_cast<float>(DPIXELS) * DIRS[edge.dirInd], color));
            window.draw(line);
        }
    }

private:
    std::vector<std::array<std::size_t, DIRS.size()>> adjList_;
    std::vector<Edge> edges_;
};
