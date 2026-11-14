// End-to-end integration test validating React frontend API contract against live C++ REST backend

async function runE2E() {
  console.log('===========================================================');
  console.log(' RUNNING END-TO-END REACT FRONTEND <-> C++ REST API TEST');
  console.log('===========================================================');

  const BASE_URL = 'http://localhost:8080';

  // 1. Health
  const healthRes = await fetch(`${BASE_URL}/api/health`);
  if (!healthRes.ok) throw new Error(`Health check failed: ${healthRes.status}`);
  const health = await healthRes.json();
  console.log(' [PASS] 1. GET /api/health ->', health.status, '| ML:', health.ml_model_loaded, '| DB:', health.database_connected);

  // 2. Menu
  const menuRes = await fetch(`${BASE_URL}/api/menu`);
  if (!menuRes.ok) throw new Error(`Menu fetch failed: ${menuRes.status}`);
  const menu = await menuRes.json();
  console.log(` [PASS] 2. GET /api/menu -> Loaded ${menu.length} food items`);

  // 3. Counters
  const countersRes = await fetch(`${BASE_URL}/api/counters`);
  if (!countersRes.ok) throw new Error(`Counters fetch failed: ${countersRes.status}`);
  const counters = await countersRes.json();
  console.log(` [PASS] 3. GET /api/counters -> Loaded ${counters.length} active service counters`);

  // 4. Place Order
  const idempotencyKey = 'e2e-txn-key-' + Date.now();
  const orderPayload = {
    student_id: 'STU_E2E_01',
    student_name: 'Tanvi Mehta',
    idempotency_key: idempotencyKey,
    items: [
      { item_id: 101, quantity: 2 },
      { item_id: 105, quantity: 1 }
    ]
  };

  const orderRes = await fetch(`${BASE_URL}/api/orders`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(orderPayload)
  });
  if (orderRes.status !== 201) throw new Error(`Order placement failed: ${orderRes.status}`);
  const placed = await orderRes.json();
  console.log(` [PASS] 4. POST /api/orders -> Order #${placed.id} assigned to Counter #${placed.assigned_counter_id} (Est: ${placed.total_prep_time_minutes}m, Total: ₹${placed.total_amount})`);

  // 5. Test Idempotency
  const retryRes = await fetch(`${BASE_URL}/api/orders`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(orderPayload)
  });
  if (retryRes.status !== 200) throw new Error(`Idempotency retry failed: ${retryRes.status}`);
  const retryOrder = await retryRes.json();
  if (retryOrder.id !== placed.id) throw new Error('Idempotency returned different order ID!');
  console.log(` [PASS] 5. Idempotent Retry -> Returned existing Order #${retryOrder.id} without duplicating`);

  // 6. Look up order
  const lookupRes = await fetch(`${BASE_URL}/api/orders/${placed.id}`);
  if (!lookupRes.ok) throw new Error(`Lookup failed: ${lookupRes.status}`);
  const lookedUp = await lookupRes.json();
  console.log(` [PASS] 6. GET /api/orders/${placed.id} -> Confirmed status: ${lookedUp.status}, payment: ${lookedUp.payment_status}`);

  // 7. Staff Authentication
  const authRes = await fetch(`${BASE_URL}/api/staff/auth`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ pin: '1234' })
  });
  if (!authRes.ok) throw new Error(`Staff login failed: ${authRes.status}`);
  const authData = await authRes.json();
  const token = authData.token;
  console.log(` [PASS] 7. POST /api/staff/auth -> Staff authenticated, received Bearer token`);

  // Verify unauthorized start is rejected with 401
  const unauthStart = await fetch(`${BASE_URL}/api/orders/${placed.id}/start`, { method: 'POST' });
  if (unauthStart.status !== 401) throw new Error(`Expected 401 Unauthorized for start without token, got ${unauthStart.status}`);
  console.log(` [PASS] 7b. POST /api/orders/${placed.id}/start (No Auth) -> Correctly rejected with HTTP 401 Unauthorized`);

  const authHeaders = {
    'Content-Type': 'application/json',
    'Authorization': `Bearer ${token}`
  };

  // 8. Accept Order (Acknowledge)
  const acceptRes = await fetch(`${BASE_URL}/api/orders/${placed.id}/accept`, {
    method: 'POST',
    headers: authHeaders
  });
  if (!acceptRes.ok) throw new Error(`Accept failed: ${acceptRes.status}`);
  const accepted = await acceptRes.json();
  if (!accepted.acknowledged) throw new Error(`Expected acknowledged: true`);
  console.log(` [PASS] 8. POST /api/orders/${placed.id}/accept -> Order acknowledged: ${accepted.acknowledged}, status remains: ${accepted.status}`);

  // 9. Confirm Counter Payment
  const payRes = await fetch(`${BASE_URL}/api/orders/${placed.id}/pay`, {
    method: 'POST',
    headers: authHeaders
  });
  if (!payRes.ok) throw new Error(`Pay failed: ${payRes.status}`);
  const paid = await payRes.json();
  if (paid.payment_status !== 'PAID') throw new Error(`Expected payment_status PAID, got ${paid.payment_status}`);
  console.log(` [PASS] 9. POST /api/orders/${placed.id}/pay -> Payment status updated to: ${paid.payment_status}`);

  // 10. Start preparation
  const startRes = await fetch(`${BASE_URL}/api/orders/${placed.id}/start`, {
    method: 'POST',
    headers: authHeaders
  });
  if (!startRes.ok) throw new Error(`Start failed: ${startRes.status}`);
  const started = await startRes.json();
  console.log(` [PASS] 10. POST /api/orders/${placed.id}/start -> Status transitioned to: ${started.status}`);

  // 11. Mark Ready
  const readyRes = await fetch(`${BASE_URL}/api/orders/${placed.id}/ready`, {
    method: 'POST',
    headers: authHeaders
  });
  if (!readyRes.ok) throw new Error(`Ready failed: ${readyRes.status}`);
  const ready = await readyRes.json();
  console.log(` [PASS] 11. POST /api/orders/${placed.id}/ready -> Status transitioned to: ${ready.status}`);

  // 12. Collect
  const collectRes = await fetch(`${BASE_URL}/api/orders/${placed.id}/collect`, {
    method: 'POST',
    headers: authHeaders
  });
  if (!collectRes.ok) throw new Error(`Collect failed: ${collectRes.status}`);
  const collected = await collectRes.json();
  console.log(` [PASS] 12. POST /api/orders/${placed.id}/collect -> Status transitioned to: ${collected.status}`);

  // 13. Analytics
  const analyticsRes = await fetch(`${BASE_URL}/api/analytics`);
  if (!analyticsRes.ok) throw new Error(`Analytics failed: ${analyticsRes.status}`);
  const analytics = await analyticsRes.json();
  console.log(` [PASS] 13. GET /api/analytics -> Logged total orders: ${analytics.total_orders_history_count}, active workload: ${analytics.total_active_workload_minutes}m`);

  console.log('===========================================================');
  console.log(' ALL END-TO-END REST API & AUTH INTEGRATION STEPS PASSED!');
  console.log('===========================================================');
}

runE2E().catch(err => {
  console.error('E2E TEST FAILED:', err);
  process.exit(1);
});
