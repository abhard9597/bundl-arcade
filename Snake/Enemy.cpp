#include "Enemy.h"

#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>
#include <cstdint>

static inline Point Step(Point p, Dir d) {
    switch (d) {
        case Dir::Up:    --p.y; break;
        case Dir::Down:  ++p.y; break;
        case Dir::Left:  --p.x; break;
        case Dir::Right: ++p.x; break;
    }
    return p;
}

static inline Point Wrap(Point p, const Board& b) {
    if (p.x < 0) p.x = b.w - 1;
    if (p.x >= b.w) p.x = 0;
    if (p.y < 0) p.y = b.h - 1;
    if (p.y >= b.h) p.y = 0;
    return p;
}

static inline int Id(const Board& b, const Point& p) { return p.y * b.w + p.x; }

static std::vector<uint8_t> BuildBlocked(const Board& b,
                                         const Snake& enemy,
                                         const Snake& player) {
    std::vector<uint8_t> blocked((size_t)b.w * (size_t)b.h, 0);
    auto mark = [&](const Snake& s) {
        for (const auto& p : s.body) {
            if (p.x >= 0 && p.y >= 0 && p.x < b.w && p.y < b.h)
                blocked[Id(b, p)] = 1;
        }
    };
    mark(player);
    mark(enemy);

    // Let the enemy step into its own tail if it is not growing.
    if (!enemy.growPending && !enemy.body.empty()) {
        const auto& tail = enemy.body.back();
        blocked[Id(b, tail)] = 0;
    }
    // Let the enemy step into the player's tail if player not growing.
    if (!player.growPending && !player.body.empty()) {
        const auto& tail = player.body.back();
        blocked[Id(b, tail)] = 0;
    }
    // Heads are always allowed as start positions.
    if (!enemy.body.empty()) blocked[Id(b, enemy.Head())] = 0;
    return blocked;
}

// Flood fill from a start cell and return reachable area size.
static int ReachableArea(const Board& b, Point start,
                         const std::vector<uint8_t>& blocked,
                         bool wrapWalls) {
    if (!wrapWalls && !b.IsInside(start)) return 0;
    if (wrapWalls) start = Wrap(start, b);
    int sid = Id(b, start);
    if (blocked[sid]) return 0;

    std::vector<uint8_t> seen(blocked.size(), 0);
    std::queue<Point> q;
    q.push(start);
    seen[sid] = 1;
    int count = 0;

    const Dir dirs[4] = {Dir::Up, Dir::Down, Dir::Left, Dir::Right};
    while (!q.empty()) {
        Point p = q.front();
        q.pop();
        ++count;
        for (Dir d : dirs) {
            Point n = Step(p, d);
            if (wrapWalls) n = Wrap(n, b);
            else if (!b.IsInside(n)) continue;
            int nid = Id(b, n);
            if (blocked[nid] || seen[nid]) continue;
            seen[nid] = 1;
            q.push(n);
        }
    }
    return count;
}

Dir EnemyAI::ChooseDir(const Board& board,
                       const Snake& enemy,
                       const Snake& player,
                       const Point& food,
                       bool wrapWalls) {
    // BFS shortest path to food.
    const Dir dirs[4] = {Dir::Up, Dir::Down, Dir::Left, Dir::Right};
    auto blocked = BuildBlocked(board, enemy, player);

    Point start = enemy.Head();
    Point goal = food;
    if (wrapWalls) {
        start = Wrap(start, board);
        goal  = Wrap(goal, board);
    }

    const int N = board.w * board.h;
    std::vector<int> prev(N, -1);
    std::vector<uint8_t> seen(N, 0);
    std::queue<Point> q;

    int sid = Id(board, start);
    q.push(start);
    seen[sid] = 1;

    while (!q.empty()) {
        Point p = q.front();
        q.pop();
        if (p.x == goal.x && p.y == goal.y) break;
        for (Dir d : dirs) {
            Point n = Step(p, d);
            if (wrapWalls) n = Wrap(n, board);
            else if (!board.IsInside(n)) continue;
            int nid = Id(board, n);
            if (seen[nid] || blocked[nid]) continue;
            seen[nid] = 1;
            prev[nid] = Id(board, p);
            q.push(n);
        }
    }

    int gid = Id(board, goal);
    if (seen[gid]) {
        // Reconstruct one step from start toward goal.
        int cur = gid;
        int parent = prev[cur];
        while (parent != -1 && parent != sid) {
            cur = parent;
            parent = prev[cur];
        }
        Point next{cur % board.w, cur / board.w};

        // Convert next cell to direction.
        Point s = start;
        // For wrap, direction ambiguity exists at borders; choose minimal delta.
        auto delta = [&](int a, int b, int size) {
            int d = b - a;
            if (wrapWalls) {
                int alt = (d > 0) ? d - size : d + size;
                if (std::abs(alt) < std::abs(d)) d = alt;
            }
            return d;
        };
        int dx = delta(s.x, next.x, board.w);
        int dy = delta(s.y, next.y, board.h);
        if (std::abs(dx) > std::abs(dy)) return (dx < 0) ? Dir::Left : Dir::Right;
        if (dy != 0) return (dy < 0) ? Dir::Up : Dir::Down;
        return enemy.dir;
    }

    // No path: pick a safe move that maximizes reachable area and avoids player proximity.
    struct Cand { Dir d; int area; int dist; };
    std::vector<Cand> cands;
    cands.reserve(4);

    for (Dir d : dirs) {
        Point n = Step(start, d);
        if (wrapWalls) n = Wrap(n, board);
        else if (!board.IsInside(n)) continue;
        int nid = Id(board, n);
        if (blocked[nid]) continue;

        int area = ReachableArea(board, n, blocked, wrapWalls);
        int dist = std::abs(n.x - player.Head().x) + std::abs(n.y - player.Head().y);
        cands.push_back({d, area, dist});
    }

    if (cands.empty()) return enemy.dir;

    // Very powerful: prioritize space, then keep distance from player.
    std::sort(cands.begin(), cands.end(), [](const Cand& a, const Cand& b) {
        if (a.area != b.area) return a.area > b.area;
        return a.dist > b.dist;
    });
    return cands.front().d;
}
