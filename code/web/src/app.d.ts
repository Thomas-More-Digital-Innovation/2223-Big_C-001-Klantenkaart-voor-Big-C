// See https://kit.svelte.dev/docs/types#app
// for information about these interfaces
// and what to do when importing types
declare namespace App {
	// interface Locals {}
	// interface PageData {}
	// interface Error {}
	interface Platform {
		env: {
			CARDS: KVNamespace;
			FREE_COFFEE_AMOUNT: string;
			API_TOKEN: string;
		};

		context: {
			waitUntil(promise: Promise<any>): void;
		};

		caches: CacheStorage & { default: Cache };
	}
}
