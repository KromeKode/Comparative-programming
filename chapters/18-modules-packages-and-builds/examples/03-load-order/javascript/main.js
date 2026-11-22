// Imports are hoisted: settings.js runs before any line of this file, wherever
// the import statement is.
import { GREETING } from "./settings.js";

console.log("main: start");
console.log(`main: the greeting is ${GREETING}`);
console.log(`main: the greeting is still ${GREETING}`);
