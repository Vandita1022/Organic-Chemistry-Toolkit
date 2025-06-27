const scene = new THREE.Scene();
const camera = new THREE.PerspectiveCamera(75, window.innerWidth/window.innerHeight, 0.1, 3000);
camera.position.z = 600;

const renderer = new THREE.WebGLRenderer({
  canvas: document.getElementById('bgCanvas'),
  antialias: true,
  alpha: true
});
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setClearColor(0x000000, 1);

// Lights
const light = new THREE.PointLight(0xffffff, 1, 0);
light.position.set(200, 200, 200);
scene.add(light);
scene.add(new THREE.AmbientLight(0x404040));

// === Create Star Texture ===
function createStarTexture() {
  const size = 64;
  const canvas = document.createElement('canvas');
  canvas.width = size;
  canvas.height = size;
  const ctx = canvas.getContext('2d');
  const gradient = ctx.createRadialGradient(size/2, size/2, 0, size/2, size/2, size/2);
  gradient.addColorStop(0, 'rgba(255,255,255,1)');
  gradient.addColorStop(0.2, 'rgba(255,255,255,0.6)');
  gradient.addColorStop(1, 'rgba(255,255,255,0)');
  ctx.fillStyle = gradient;
  ctx.beginPath();
  ctx.arc(size/2, size/2, size/2, 0, Math.PI * 2);
  ctx.fill();
  return new THREE.CanvasTexture(canvas);
}

// === Star Field ===
const starCount = 200000;
const starGeometry = new THREE.BufferGeometry();
const starPositions = [];

for (let i = 0; i < starCount; i++) {
  starPositions.push(
    (Math.random() - 0.5) * 4000,
    (Math.random() - 0.5) * 4000,
    (Math.random() - 0.5) * 4000
  );
}

starGeometry.setAttribute('position', new THREE.Float32BufferAttribute(starPositions, 3));
const starMaterial = new THREE.PointsMaterial({
  size: 3,
  map: createStarTexture(),
  transparent: true,
  alphaTest: 0.1,
  depthWrite: false,
});
const stars = new THREE.Points(starGeometry, starMaterial);
scene.add(stars);

// === CH₄ Molecule System ===
const hydrogenMaterial = new THREE.MeshPhongMaterial({ color: 0xffffff });
const bondMaterial = new THREE.LineBasicMaterial({ color: 0x888888 });
const moleculeGroup = new THREE.Group();

const tetrahedralOffsets = [
  new THREE.Vector3(1, 1, 1),
  new THREE.Vector3(-1, -1, 1),
  new THREE.Vector3(-1, 1, -1),
  new THREE.Vector3(1, -1, -1)
];

function createMolecule(position) {
  const group = new THREE.Group();
  const carbonMaterial = new THREE.MeshPhongMaterial({
    map: createStarTexture(),
    transparent: true,
    depthWrite: false,
    color: 0xffffff
  });

  const carbon = new THREE.Mesh(new THREE.SphereGeometry(5, 16, 16), carbonMaterial);
  group.add(carbon);

  tetrahedralOffsets.forEach(offset => {
    const hydrogenPos = offset.clone().normalize().multiplyScalar(20);
    const hydrogen = new THREE.Mesh(new THREE.SphereGeometry(2, 12, 12), hydrogenMaterial);
    hydrogen.position.copy(hydrogenPos);
    group.add(hydrogen);

    const bondGeo = new THREE.BufferGeometry().setFromPoints([new THREE.Vector3(0, 0, 0), hydrogenPos]);
    const bond = new THREE.Line(bondGeo, bondMaterial);
    group.add(bond);
  });

  group.position.copy(position);
  group.userData.velocity = new THREE.Vector3(
    (Math.random() - 0.5) * 0.8,
    (Math.random() - 0.5) * 0.8,
    (Math.random() - 0.5) * 0.8
  );
  moleculeGroup.add(group);
}

for (let i = 0; i < 50; i++) {
  const pos = new THREE.Vector3(
    (Math.random() - 0.5) * 1500,
    (Math.random() - 0.5) * 1500,
    (Math.random() - 0.5) * 1500
  );
  createMolecule(pos);
}

scene.add(moleculeGroup);

// === Mouse Interaction ===
let mouseX = 0, mouseY = 0;
let targetX = 0, targetY = 0;
const windowHalfX = window.innerWidth / 2;
const windowHalfY = window.innerHeight / 2;

document.addEventListener('mousemove', (event) => {
  mouseX = event.clientX - windowHalfX;
  mouseY = event.clientY - windowHalfY;
});

// === Animate ===
function animate() {
  requestAnimationFrame(animate);

  targetX = mouseX * 0.002;
  targetY = mouseY * 0.002;

  stars.rotation.y += 0.0015;
  moleculeGroup.rotation.y += 0.003 * (targetX - moleculeGroup.rotation.y);
  moleculeGroup.rotation.x += 0.003 * (targetY - moleculeGroup.rotation.x);

  moleculeGroup.children.forEach(group => {
    group.rotation.x += 0.005;
    group.rotation.y += 0.005;
    group.position.add(group.userData.velocity);

    ['x', 'y', 'z'].forEach(axis => {
      if (group.position[axis] > 1000 || group.position[axis] < -1000) {
        group.userData.velocity[axis] *= -1;
      }
    });
  });

  renderer.render(scene, camera);
}
animate();

// === Handle Resize ===
window.addEventListener('resize', () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
});
