const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const cors = require('cors');

const app = express();
app.use(cors());
const server = http.createServer(app);
const io = new Server(server, { cors: { origin: "*" } });

// ข้อมูลโลกในเกม (ต้นไม้, หิน, แร่, จุดตกปลา, มอนสเตอร์, บอสดันเจี้ยน)
const worldObjects = [
    { id: 'tree1', type: 'tree', name: 'ต้นไม้', x: 200, y: 150, hp: 30, maxHp: 30, drop: 'wood', exp: 10 },
    { id: 'tree2', type: 'tree', name: 'ต้นไม้', x: 260, y: 220, hp: 30, maxHp: 30, drop: 'wood', exp: 10 },
    { id: 'rock1', type: 'rock', name: 'ก้อนหิน', x: 450, y: 150, hp: 40, maxHp: 40, drop: 'stone', exp: 15 },
    { id: 'ore1',  type: 'ore',  name: 'แร่เหล็ก', x: 520, y: 200, hp: 50, maxHp: 50, drop: 'iron', exp: 25 },
    { id: 'fish1', type: 'fish', name: 'บ่อตกปลา', x: 150, y: 400, hp: 10, maxHp: 10, drop: 'fish', exp: 15 },
    { id: 'mob1',  type: 'mob',  name: 'สไลม์ (Lv.1)', x: 600, y: 380, hp: 50, maxHp: 50, drop: 'wood', exp: 30 },
    // โซนดันเจี้ยน (อยู่มุมขวาล่างของแผนที่)
    { id: 'boss1', type: 'boss', name: '👑 บอสดันเจี้ยน (Lv.5)', x: 850, y: 480, hp: 200, maxHp: 200, drop: 'iron', exp: 150 }
];

const players = {};

io.on('connection', (socket) => {
    console.log('ผู้เล่นเชื่อมต่อ:', socket.id);

    // สร้างข้อมูลเริ่มต้นให้ผู้เล่นใหม่
    players[socket.id] = {
        id: socket.id,
        name: 'ผู้กล้า_' + socket.id.substring(0, 4),
        x: 100, y: 100,
        lv: 1, exp: 0, maxExp: 50,
        atk: 10,
        inv: { wood: 0, stone: 0, iron: 0, fish: 0 }
    };

    // ส่งข้อมูลทั้งหมดให้ผู้เล่นที่เพิ่งเข้ามา
    socket.emit('init', { id: socket.id, players, worldObjects });
    socket.broadcast.emit('playerJoined', players[socket.id]);

    // 1. อัปเดตการเดิน
    socket.on('move', (pos) => {
        if (players[socket.id]) {
            players[socket.id].x = pos.x;
            players[socket.id].y = pos.y;
            socket.broadcast.emit('playerMoved', { id: socket.id, x: pos.x, y: pos.y });
        }
    });

    // 2. ระบบฟาร์ม (ตัดไม้ / ขุดหิน / ขุดแร่ / ตกปลา / ตีมอน / ตีบอส)
    socket.on('interact', (objId) => {
        const p = players[socket.id];
        const obj = worldObjects.find(o => o.id === objId);
        if (!p || !obj || obj.hp <= 0) return;

        // เช็กระยะห่างว่ายืนใกล้เป้าหมายไหม
        const dist = Math.hypot(p.x - obj.x, p.y - obj.y);
        if (dist > 90) return;

        obj.hp -= p.atk;
        io.emit('updateObject', obj);

        // เมื่อเก็บเกี่ยวสำเร็จ หรือ มอนสเตอร์ตาย
        if (obj.hp <= 0) {
            const amount = obj.type === 'boss' ? 5 : 1;
            p.inv[obj.drop] += amount;
            p.exp += obj.exp;

            // เช็กอัปเลเวล
            if (p.exp >= p.maxExp) {
                p.lv += 1;
                p.exp -= p.maxExp;
                p.maxExp = Math.floor(p.maxExp * 1.5);
                p.atk += 5;
            }
            socket.emit('updateSelf', p);

            // เกิดใหม่ใน 5 วินาที (Respawn)
            setTimeout(() => {
                obj.hp = obj.maxHp;
                io.emit('updateObject', obj);
            }, 5000);
        }
    });

    // 3. ระบบคราฟของ
    socket.on('craft', (recipe) => {
        const p = players[socket.id];
        if (!p) return;

        if (recipe === 'stone_sword' && p.inv.wood >= 3 && p.inv.stone >= 2) {
            p.inv.wood -= 3;
            p.inv.stone -= 2;
            p.atk += 15;
            socket.emit('updateSelf', p);
            socket.emit('msg', 'คราฟ [ดาบหิน] สำเร็จ! พลังโจมตี +15');
        } else if (recipe === 'iron_sword' && p.inv.iron >= 3 && p.inv.wood >= 2) {
            p.inv.iron -= 3;
            p.inv.wood -= 2;
            p.atk += 35;
            socket.emit('updateSelf', p);
            socket.emit('msg', 'คราฟ [ดาบเหล็ก] สำเร็จ! พลังโจมตี +35');
        } else if (recipe === 'grilled_fish' && p.inv.fish >= 1) {
            p.inv.fish -= 1;
            p.exp += 25;
            if (p.exp >= p.maxExp) { p.lv++; p.exp -= p.maxExp; p.atk += 5; }
            socket.emit('updateSelf', p);
            socket.emit('msg', 'กิน [ปลาย่าง] ได้รับ EXP +25!');
        } else {
            socket.emit('msg', 'วัตถุดิบไม่พอ!');
        }
    });

    socket.on('disconnect', () => {
        delete players[socket.id];
        io.emit('playerLeft', socket.id);
    });
});

server.listen(3000, () => {
    console.log('เซิร์ฟเวอร์เกมเปิดทำงานแล้วที่ http://localhost:3000');
});
