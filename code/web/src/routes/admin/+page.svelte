<script lang="ts">
	import type { ActionData, PageData } from './$types';

	export let data: PageData;
	export let form: ActionData;

	const compareReportDate = (a: string, b: string): number => {
		const splittedA = a.split('/');
		const splittedB = b.split('/');

		const yearDiff = parseInt(splittedA[0]) - parseInt(splittedB[0]);
		if (yearDiff !== 0) return -yearDiff;

		const monthDiff = parseInt(splittedA[1]) - parseInt(splittedB[1]);
		return -monthDiff;
	};
</script>

<div class="py-4 px-4 flex flex-col items-center">
	<div class="w-full grid grid-cols-1 md:grid-cols-2 gap-2">
		<h1 class="text-2xl col-span-full">Admin</h1>
		<form method="post">
			<h2 class="text-lg">Settings</h2>
			{#if form?.success}
				<div class="alert alert-success shadow-lg">
					<div>
						<svg
							xmlns="http://www.w3.org/2000/svg"
							class="stroke-current flex-shrink-0 h-6 w-6"
							fill="none"
							viewBox="0 0 24 24"
							><path
								stroke-linecap="round"
								stroke-linejoin="round"
								stroke-width="2"
								d="M9 12l2 2 4-4m6 2a9 9 0 11-18 0 9 9 0 0118 0z"
							/></svg
						>
						<span
							>The amount of coffees has been updated! Note that these changes may take up to 60
							seconds to propagate.</span
						>
					</div>
				</div>
			{/if}
			<div class="form-control w-full">
				<label class="label" for="freeCoffeeAmount">
					<span class="label-text">Amount till freebie?</span>
				</label>
				<input
					type="number"
					id="freeCoffeeAmount"
					name="freeCoffeeAmount"
					placeholder="Type here"
					class="input input-bordered w-full"
					min="1"
					value={form?.freeCoffeeAmount || data.freeCoffeeAmount}
				/>
				{#if form?.missing}
					<label class="label" for="freeCoffeeAmount">
						<span class="label-text-alt text-error">The amount of coffees is required.</span>
					</label>
				{/if}
				<label class="label" for="freeCoffeeAmount">
					<span class="label-text-alt"
						>Choose the amount of coffees a person needs before they are elegible to get a free
						coffee.</span
					>
				</label>
			</div>
			<button type="submit" class="btn btn-primary btn-block">Save</button>
		</form>
		<div>
			<h2 class="text-lg">Reports</h2>
			<div class="overflow-x-auto">
				<table class="table table-zebra w-full">
					<thead>
						<tr>
							<th>Date</th>
							<th>Amount</th>
						</tr>
					</thead>
					<tbody>
						{#each data.reports.sort((a, b) => compareReportDate(a.date, b.date)) as report}
							<tr>
								<th>{report.date}</th>
								<td>{report.amount}</td>
							</tr>
						{/each}
					</tbody>
				</table>
			</div>
		</div>
	</div>
</div>
