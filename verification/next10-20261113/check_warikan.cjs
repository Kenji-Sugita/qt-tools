const assert = require('node:assert/strict');
const path = require('node:path');
const fs = require('node:fs');
const root = path.resolve(__dirname, '../..');
const app = require(path.join(root, 'tools/warikan/src/app.js'));
const tests = [];
function check(name, run) { run(); tests.push({name, passed:true}); }
const session = {totalAmount:50000, roundingUnit:500, groups:[
 {id:'a', name:'多め', ratio:1.5, count:2},
 {id:'b', name:'普通', ratio:1, count:6},
 {id:'c', name:'少なめ', ratio:0.5, count:2}
]};
const calculated = app.calculateWarikan(session);
check('representative per-person amounts', () => assert.deepEqual(calculated.result.rows.map(r=>r.amountPerPerson), [7500,5000,2500]));
check('total and balance', () => {assert.equal(calculated.errors.length,0);assert.equal(calculated.result.totalCalculated,50000);assert.equal(calculated.result.isBalanced,true);});
const members = app.createCollectionMembers(calculated.result);
check('ten participants', () => assert.equal(members.length,10));
check('initial collection summary', () => assert.deepEqual(app.summarizeCollection(members), {plannedTotal:50000,collectedTotal:0,uncollectedTotal:50000,collectedCount:0,uncollectedCount:10}));
check('all collected', () => {members.forEach(m=>m.collected=true);assert.equal(app.summarizeCollection(members).collectedTotal,50000);});
check('reject invalid total', () => assert.ok(app.calculateWarikan({...session,totalAmount:0}).errors.length));
check('reject invalid rounding unit', () => assert.ok(app.calculateWarikan({...session,roundingUnit:0}).errors.length));
check('no persistence API in implementation', () => assert.doesNotMatch(fs.readFileSync(path.join(root,'tools/warikan/src/app.js'),'utf8'), /localStorage|sessionStorage|indexedDB/));
console.log(JSON.stringify({scope:'Node.js logic and persistence API inspection; not a browser GUI or other-OS test', tests},null,2));
