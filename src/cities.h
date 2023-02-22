
float SquareDistance(WindXy a, WindXy b) {
    return Dot(a - b, a - b);
}

//float Distance(int from, int to) {
//    return std::sqrt(SquareDistance(vertexData_[from].coords, vertexData_[to].coords));
//}

struct Edge {
    int to;
    int id;
};

struct EdgeData {
    sf::RectangleShape strip;
};

struct VertexData {
    WindXy coords;
    std::unique_ptr<sf::CircleShape> point;
};

class CitiesGame {
public:
    CitiesGame()
        : graph_(NUM_VERTICES)
        , vertexData_(NUM_VERTICES)
    {
        static_assert(WIDTH % NODES_DIST == 0);
        static_assert(HEIGHT % NODES_DIST == 0);

        for (int node = 0; node < NUM_VERTICES; ++node) {
            vertexData_[node].coords = {
                static_cast<float>(node % (WIDTH / NODES_DIST) * NODES_DIST - NODE_RADIUS),
                static_cast<float>(node / (WIDTH / NODES_DIST) * NODES_DIST - NODE_RADIUS),
            };
        }

        player_.setPosition(0, 0);
        player_.setFillColor(GREY);
        player_.setSize(sf::Vector2f(100, 100));
    }

    void HandleInput(const sf::Event& event) {
        if (event.type == sf::Event::EventType::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Button::Left) {
                Choose(event.mouseButton.x, event.mouseButton.y);
            } else if (event.mouseButton.button == sf::Mouse::Button::Right) {
                chosen_ = -1;
            }
        }
    }

    void Update(sf::RenderWindow& window) {
    }

    void Render(sf::RenderWindow& window, float part) {
        for (std::size_t node = 0; node < graph_.size(); ++node) {
            if (vertexData_[node].point) {
                window.draw(*vertexData_[node].point);
            }
            for (auto edge : graph_[node]) {
                window.draw(edgeData_[edge.id].strip);
            }
        }
    }

private:
    void Choose(int x, int y) {
        int xInd = std::lround(x / static_cast<double>(NODES_DIST));
        int yInd = std::lround(y / static_cast<double>(NODES_DIST));
        int node = yInd * WIDTH / NODES_DIST + xInd;
        if (chosen_ == -1) {
            chosen_ = node;
            return;
        }
        if (chosen_ != node) {
            auto id = AddEdgeData(chosen_, node);
            if (id != -1) {
                graph_[chosen_].push_back({node, id});
                AddVertexData(chosen_);
                AddVertexData(node);
                chosen_ = node;
            }
        }
    }

    int AddEdgeData(int from, int to) {
        for (auto edge : graph_[from]) {
            if (edge.to == to) {
                return -1;
            }
        }
        int id = edgeData_.size();
        auto& data = edgeData_.emplace_back();
        auto direction = vertexData_[to].coords - vertexData_[from].coords;
        data.strip.setSize({
            NODE_RADIUS,
            std::abs(direction),
        });
        data.strip.setPosition(vertexData_[from].coords);
        data.strip.setFillColor(GREY);
        data.strip.rotate(std::arg(direction) - 90);

        move strip to vertex's center

        return id;
    }

    void AddVertexData(int node) {
        auto& point = vertexData_[node].point;
        if (!point) {
            point = std::make_unique<sf::CircleShape>(NODE_RADIUS);
            point->setPosition(vertexData_[node].coords);
            point->setFillColor(sf::Color::Cyan);
        }
    }

    sf::RectangleShape player_;

    static constexpr int NODES_DIST = 50;
    static constexpr int NODE_RADIUS = NODES_DIST / 2 - 1;
    static constexpr int NUM_VERTICES = WIDTH * HEIGHT / NODES_DIST / NODES_DIST;
    float roadWidth_ = 60;
    int chosen_ = -1;
    std::vector<std::vector<Edge>> graph_;
    std::vector<VertexData> vertexData_;
    std::vector<EdgeData> edgeData_;
};
