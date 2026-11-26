const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const cors = require('cors');

const app = express();
app.use(cors());
const server = http.createServer(app);
const io = new Server(server, { cors: { origin: '*' } });

const PORT = process.env.PORT || 3000;
const MOVE_LIMIT = 300;
const INTERACT_RANGE = 90;
const RESPAWN_MS = 5000;

const worldObjects = [
    { id: 'tree1', type: 'tree', name: 'ต้นไม้', x: 200, y: 150, hp: 30, maxHp: 30, drop: 'wood', exp: 10 },
    { id: 'tree2', type: 'tree', name: 'ต้นไม้', x: 260, y: 220, hp: 30, maxHp: 30, drop: 'wood', exp: 10 },
    { id: 'rock1', type: 'rock', name: 'ก้อนหิน', x: 450, y: 150, hp: 40, maxHp: 40, drop: 'stone', exp: 15 },
    { id: 'ore1',  type: 'ore',  name: 'แร่เหล็ก', x: 520, y: 200, hp: 50, maxHp: 50, drop: 'iron', exp: 25 },
    { id: 'fish1', type: 'fish', name: 'บ่อตกปลา', x: 150, y: 400, hp: 10, maxHp: 10, drop: 'fish', exp: 15 },
    { id: 'mob1',  type: 'mob',  name: 'สไลม์ (Lv.1)', x: 600, y: 380, hp: 50, maxHp: 50, drop: 'wood', exp: 30 },
    { id: 'boss1', type: 'boss', name: '👑 บอสดันเจี้ยน (Lv.5)', x: 850, y: 480, hp: 200, maxHp: 200, drop: 'iron', exp: 150 }
];

const players = {};
const respawnTimers = {};

function emptyInv() {
    return { wood: 0, stone: 0, iron: 0, fish: 0 };
}

function applyExp(p, amount) {
    p.exp += amount;
    while (p.exp >= p.maxExp) {
        p.lv += 1;
        p.exp -= p.maxExp;
        p.maxExp = Math.floor(p.maxExp * 1.5);
        p.atk += 5;
    }
}

io.on('connection', (socket) => {
    console.log('ผู้เล่นเชื่อมต่อ:', socket.id);

    players[socket.id] = {
        id: socket.id,
        name: 'ผู้กล้า_' + socket.id.substring(0, 4),
        x: 100, y: 100,
        lv: 1, exp: 0, maxExp: 50,
        atk: 10,
        inv: emptyInv()
    };

    socket.emit('init', { id: socket.id, players, worldObjects });
    socket.broadcast.emit('playerJoined', players[socket.id]);

    socket.on('move', (pos) => {
        const p = players[socket.id];
        if (!p || typeof pos?.x !== 'number' || typeof pos?.y !== 'number') return;

        const dist = Math.hypot(pos.x - p.x, pos.y - p.y);
        if (dist > MOVE_LIMIT) return;

        p.x = pos.x;
        p.y = pos.y;
        socket.broadcast.emit('playerMoved', { id: socket.id, x: p.x, y: p.y });
    });

    socket.on('interact', (objId) => {
        const p = players[socket.id];
        const obj = worldObjects.find(o => o.id === objId);
        if (!p || !obj || obj.hp <= 0) return;

        if (Math.hypot(p.x - obj.x, p.y - obj.y) > INTERACT_RANGE) return;

        obj.hp -= p.atk;
        io.emit('updateObject', obj);

        if (obj.hp > 0) return;

        obj.hp = 0;
        const amount = obj.type === 'boss' ? 5 : 1;
        if (p.inv[obj.drop] !== undefined) p.inv[obj.drop] += amount;
        applyExp(p, obj.exp);
        socket.emit('updateSelf', p);

        if (respawnTimers[obj.id]) clearTimeout(respawnTimers[obj.id]);
        respawnTimers[obj.id] = setTimeout(() => {
            obj.hp = obj.maxHp;
            io.emit('updateObject', obj);
            delete respawnTimers[obj.id];
        }, RESPAWN_MS);
    });

    socket.on('craft', (recipe) => {
        const p = players[socket.id];
        if (!p) return;

        const recipes = {
            stone_sword: {
                check: () => p.inv.wood >= 3 && p.inv.stone >= 2,
                run: () => { p.inv.wood -= 3; p.inv.stone -= 2; p.atk += 15; },
                msg: 'คราฟ [ดาบหิน] สำเร็จ! พลังโจมตี +15'
            },
            iron_sword: {
                check: () => p.inv.iron >= 3 && p.inv.wood >= 2,
                run: () => { p.inv.iron -= 3; p.inv.wood -= 2; p.atk += 35; },
                msg: 'คราฟ [ดาบเหล็ก] สำเร็จ! พลังโจมตี +35'
            },
            grilled_fish: {
                check: () => p.inv.fish >= 1,
                run: () => { p.inv.fish -= 1; applyExp(p, 25); },
                msg: 'กิน [ปลาย่าง] ได้รับ EXP +25!'
            }
        };

        const r = recipes[recipe];
        if (!r) return socket.emit('msg', 'ไม่รู้จักสูตรนี้');
        if (!r.check()) return socket.emit('msg', 'วัตถุดิบไม่พอ!');

        r.run();
        socket.emit('updateSelf', p);
        socket.emit('msg', r.msg);
    });

    socket.on('disconnect', () => {
        delete players[socket.id];
        io.emit('playerLeft', socket.id);
        console.log('ผู้เล่นออก:', socket.id);
    });
});

server.listen(PORT, () => {
    console.log(`เซิร์ฟเวอร์เกมเปิดทำงานแล้วที่ http://localhost:${PORT}`);
});