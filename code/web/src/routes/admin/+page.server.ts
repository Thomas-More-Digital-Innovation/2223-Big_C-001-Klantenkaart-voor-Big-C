import { invalid } from '@sveltejs/kit';
import type { Actions, PageServerLoad } from './$types';

export const load: PageServerLoad = async ({ platform }) => {
	if (!platform) {
		return {
			freeCoffeeAmount: 10
		};
	}

	const freeCoffeeAmount = (await platform.env.SETTINGS.get('freeCoffeeAmount')) || '0';

	return {
		freeCoffeeAmount: parseInt(freeCoffeeAmount)
	};
};

export const actions: Actions = {
	default: async ({ request, platform }) => {
		const data = await request.formData();
		const freeCoffeeAmount = data.get('freeCoffeeAmount');

		if (!freeCoffeeAmount) {
			return invalid(400, { freeCoffeeAmount, missing: true });
		}

		if (platform) {
			await platform.env.SETTINGS.put('freeCoffeeAmount', freeCoffeeAmount.toString());
		}

		return { success: true };
	}
};
