// Map of canonical menu item names to high-resolution web-optimized photography
export const ITEM_IMAGE_MAP: Record<string, string> = {
  'poha':               '/images/menu/poha.jpg',
  'fruit plate':        '/images/menu/fruit-plate.jpg',
  'patties':            '/images/menu/patties.jpg',
  'uttapam':            '/images/menu/uttapam.jpg',
  'samosa':             '/images/menu/samosa.jpg',
  'samosa (2 pcs)':     '/images/menu/samosa.jpg',
  'medu vada':          '/images/menu/medu-vada.jpg',
  'vada pav':           '/images/menu/vada-pav.jpg',
  'idli sambhar':       '/images/menu/idli-sambhar.jpg',
  'sabudana vada':      '/images/menu/sabudana-vada.jpg',
  'sabudana khichdi':   '/images/menu/sabudana-khichdi.jpg',
  'aloo paratha':       '/images/menu/aloo-paratha.jpg',
  'paneer paratha':     '/images/menu/paneer-paratha.jpg',
  'plain dosa':         '/images/menu/plain-dosa.jpg',
  'masala dosa':        '/images/menu/masala-dosa.jpg',
  'plain sandwich':     '/images/menu/plain-sandwich.jpg',
  'paneer sandwich':    '/images/menu/paneer-sandwich.jpg',
  'fried rice':         '/images/menu/fried-rice.jpg',
  'veg fried rice':     '/images/menu/fried-rice.jpg',
  'kolam rice':         '/images/menu/kolam-rice.jpg',
  'noodles':            '/images/menu/noodles.jpg',
  'spicy noodles':      '/images/menu/spicy-noodles.jpg',
  'chikoo shake':       '/images/menu/chikoo-shake.jpg',
  'mango shake':        '/images/menu/mango-shake.jpg',
  'banana shake':       '/images/menu/banana-shake.jpg',
  'pineapple shake':    '/images/menu/pineapple-shake.jpg',
  'anjeer shake':       '/images/menu/anjeer-shake.jpg',
  'strawberry shake':   '/images/menu/strawberry-shake.jpg',
  'cold coffee':        '/images/menu/cold-coffee.jpg',
  'thick cold coffee':  '/images/menu/thick-cold-coffee.jpg',
  'butterscotch shake': '/images/menu/butterscotch-shake.jpg',
};

export function getItemImage(name: string): string | null {
  const n = name.toLowerCase().trim();
  if (ITEM_IMAGE_MAP[n]) return ITEM_IMAGE_MAP[n];
  for (const [key, path] of Object.entries(ITEM_IMAGE_MAP)) {
    if (n.includes(key) || key.includes(n)) return path;
  }
  return null;
}
