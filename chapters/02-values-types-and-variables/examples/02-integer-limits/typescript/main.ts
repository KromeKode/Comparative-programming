const i32Max: number = 2 ** 31 - 1;
console.log(`int32 max: ${i32Max}`);
console.log(`int32 max + 1: ${i32Max + 1}`); // numbers are 64-bit floats, so no overflow yet
console.log(`int32 max + 1, as a 32-bit int: ${(i32Max + 1) | 0}`); // | works on 32-bit ints

const safe: number = Number.MAX_SAFE_INTEGER; // 2^53 - 1
console.log(`largest safe integer: ${safe}`);
console.log(`largest safe integer + 2: ${safe + 2}`); // off by one, and no error

const i64Max: bigint = 2n ** 63n - 1n; // BigInt: whole numbers of any size
console.log(`int64 max (BigInt): ${i64Max}`);
console.log(`int64 max + 1 (BigInt): ${i64Max + 1n}`);
