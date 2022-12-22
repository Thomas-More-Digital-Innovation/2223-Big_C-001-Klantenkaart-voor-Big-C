import { error, json, type RequestHandler } from '@sveltejs/kit';
import status from 'http-status';

export const POST: RequestHandler = async ({ params, request, platform }) => {
	const cardUID = params.cardUID;
	if (!cardUID) throw error(status.BAD_REQUEST, 'CardUID missing');

	if (!request.headers.has('x-api-key')) throw error(status.BAD_REQUEST, 'Missing X-API-Key');
	if (request.headers.get('x-api-key') !== platform.env.API_TOKEN)
		throw error(status.UNAUTHORIZED, 'Invalid API key');

	const freeCoffeeAmount = parseInt((await platform.env.SETTINGS.get('freeCoffeeAmount')) || '0');

	let value = parseInt((await platform.env.CARDS.get(cardUID)) || '0');

	const today = new Date();
	const monthlyKey = `monthly:${today.getUTCFullYear()}/${today.getUTCMonth()}`;
	let monthlyValue = parseInt((await platform.env.SETTINGS.get(monthlyKey)) || '0');

	value += 1;
	monthlyValue += 1;

	await platform.env.SETTINGS.put(monthlyKey, monthlyValue.toString());

	if (value >= freeCoffeeAmount) {
		await platform.env.CARDS.put(cardUID, '0');
	} else {
		await platform.env.CARDS.put(cardUID, value.toString());
	}

	return json(
		{
			status: status[status.CREATED],
			data: {
				count: value,
				countToGo: freeCoffeeAmount - value,
				freeCoffee: value === freeCoffeeAmount
			}
		},
		{ status: status.CREATED }
	);
};
