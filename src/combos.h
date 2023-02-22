
struct AttackState {
    virtual ~AttackState() = default;
    virtual AttackState* HandleInput(const sf::Event& event) = 0;
};

struct FirstAttack : public AttackState {
    virtual AttackState* HandleInput(const sf::Event& event);

    bool canBeStopped_ = false;
    bool canBeContinued_ = false;
} firstAttack;

struct SecondAttack : public AttackState {
    virtual AttackState* HandleInput(const sf::Event& event) {}

    bool canBeStopped_ = false;
    bool canBeContinued_ = false;
} secondAttack;

AttackState* FirstAttack::HandleInput(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Key::J && canBeContinued_) {
            return &secondAttack;
        }
        if (event.key.code == sf::Keyboard::Key::S && canBeStopped_) {
            return nullptr;
        }
    }
    return this;
}

class CombosGame {
public:
    CombosGame() {
        player_.setPosition(0, 0);
        player_.setFillColor(GREY);
        player_.setSize(sf::Vector2f(100, 100));
    }

    void HandleInput(const sf::Event& event) {
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Key::J) {
            if (attackState_) {
                attackState_ = attackState_->HandleInput(event);
            } else {
                attackState_ = &firstAttack;
            }
            return;
        }
        if (attackState_) {
            attackState_ = attackState_->HandleInput(event);
            return;
        }
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Key::A) {
                dir_.x = -1;
            } else if (event.key.code == sf::Keyboard::Key::D) {
                dir_.x = 1;
            } else if (event.key.code == sf::Keyboard::Key::W) {
                dir_.y = -1;
            } else if (event.key.code == sf::Keyboard::Key::S) {
                dir_.y = 1;
            }
        } else if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::Key::A) {
                dir_.x = 0;
            } else if (event.key.code == sf::Keyboard::Key::D) {
                dir_.x = 0;
            } else if (event.key.code == sf::Keyboard::Key::W) {
                dir_.y = 0;
            } else if (event.key.code == sf::Keyboard::Key::S) {
                dir_.y = 0;
            }
        }
    }

    void Update(sf::RenderWindow& window) {
        player_.move(dir_.x, dir_.y);
    }

    void Render(sf::RenderWindow& window, float part) {
        window.draw(player_);
    }

private:
    sf::Vector2i origin_;
    sf::Vector2i dir_;
    sf::RectangleShape player_;
    int side_;
    AttackState* attackState_ = nullptr;
};
