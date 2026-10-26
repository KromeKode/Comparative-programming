const sum = 0.1 + 0.2;

console.log(`0.1 + 0.2 = ${sum}`);
console.log(`equal to 0.3? ${sum === 0.3 ? "yes" : "no"}`);
console.log(`close to 0.3? ${Math.abs(sum - 0.3) < 1e-9 ? "yes" : "no"}`);
