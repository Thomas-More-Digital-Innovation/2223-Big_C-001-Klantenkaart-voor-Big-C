import { invalid } from '@sveltejs/kit';
import type { Actions, PageServerLoad } from './$types';

export interface Report {
	date: string;
	amount: number;
}

export interface AdminPageData {
	freeCoffeeAmount: number;
	reports: Report[];
}

export const load: PageServerLoad = async ({ platform }): Promise<AdminPageData> => {
	if (!platform) {
		return {
			freeCoffeeAmount: 10,
			reports: [
				{
					date: '2023/01',
					amount: 6
				},
				{
					date: '2023/02',
					amount: 1
				},
				{
					date: '2022/10',
					amount: 8
				},
				{
					date: '2022/11',
					amount: 9
				},
				{
					date: '2022/12',
					amount: 4
				}
			]
		};
	}

	const freeCoffeeAmount = (await platform.env.SETTINGS.get('freeCoffeeAmount')) || '0';

	const reportKeys = await platform.env.SETTINGS.list<string>({ prefix: 'monthly:' });
	const reports: Report[] = [];

	for (const reportKey of reportKeys.keys) {
		const reportAmount = (await platform.env.SETTINGS.get(reportKey.name)) || '0';

		reports.push({
			date: reportKey.name.replace('monthly:', ''),
			amount: parseInt(reportAmount)
		});
	}

	return {
		freeCoffeeAmount: parseInt(freeCoffeeAmount),
		reports
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
